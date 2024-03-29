#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <sie/sie.h>
#include "ipc.h"
#include "path_stack.h"
#include "procs/procs.h"
#include "menu/options.h"

#define MAIN_CSM_NAME "Files"

typedef struct {
    GUI gui;
} MAIN_GUI;

typedef struct {
    CSM_RAM csm;
    MAIN_GUI *main_gui;
} MAIN_CSM;


static int _OnKey(MAIN_GUI *data, GUI_MSG *msg);
void UpdateCSMname();

/**********************************************************************************************************************/
int DEFAULT_DISK;
char *DIR_TEMPLATES;

static const int minus11 = -11;
unsigned short maincsm_name_body[140];
RECT canvas = {0, 0, 0, 0};

SIE_GUI_SURFACE *SURFACE;
SIE_MENU_LIST *MENU;
SIE_FILE *FILES;
SIE_FILE *CURRENT_FILE;
SIE_FILE *SELECTED_FILES;
SIE_FILE *COPY_FILES, *MOVE_FILES;
path_stack_t *PATH_STACK;
SIE_GUI_STACK *GUI_STACK;

unsigned int SORT = SORT_BY_NAME_ASC;
unsigned int SHOW_HIDDEN_FILES = 1;

/**********************************************************************************************************************/

SIE_FILE *InitRootFiles() {
    const int count = Sie_FS_MMCardExists() ? 4 : 3;
    const char *names[] = {"0:", "1:", "2:", "4:"};
    const char *aliases[] = {"Data", "Cache", "Config", "MMCard"};
    const int attrs[] = {SIE_FS_FA_VOLUME,
                         SIE_FS_FA_HIDDEN + SIE_FS_FA_VOLUME,
                         SIE_FS_FA_HIDDEN + SIE_FS_FA_VOLUME,
                         SIE_FS_FA_VOLUME,
                         };

    size_t len;
    SIE_FILE *prev = NULL;
    SIE_FILE *current = NULL;
    for (int i = 0; i < count; i++) {
        current = malloc(sizeof(SIE_FILE));
        zeromem(current, sizeof(SIE_FILE));
        current->dir_name = malloc(1);
        current->dir_name[0] = '\0';
        len = strlen(names[i]);
        current->file_name = malloc(len + 1);
        strcpy(current->file_name, names[i]);
        len = strlen(aliases[i]);
        current->alias = malloc(len + 1);
        strcpy(current->alias, aliases[i]);
        current->file_attr = attrs[i];
        if (prev) {
            current->prev = prev;
            prev->next = current;
        }
        prev = current;
    }
    return Sie_FS_GetFirstFile(current);
}

SIE_MENU_LIST_ITEM *InitItems(SIE_FILE *top, unsigned int *count) {
    static char color_hidden[] = SIE_COLOR_TEXT_DISABLED;

    SIE_MENU_LIST_ITEM *items = NULL;

    SIE_FILE *file = top;
    unsigned int i = 0;
    while (file) {
        if (!SHOW_HIDDEN_FILES && (file->file_attr & SIE_FS_FA_HIDDEN)) { // pass hidden files
            file = file->next;
            continue;
        }

        SIE_MENU_LIST_ITEM *item;
        items = realloc(items, sizeof(SIE_MENU_LIST_ITEM) * (i + 1));
        item = &(items[i]);
        zeromem(item, sizeof(SIE_MENU_LIST_ITEM));

        // icon
        if (file->file_attr & SIE_FS_FA_VOLUME) {
            char name[8];
            if (strcmp(file->file_name, "4:") == 0) {
                strcpy(name, "mmcard");
            } else {
                strcpy(name, "disk");
            }
            item->icon = Sie_Resources_LoadIMGHDR(SIE_RESOURCES_TYPE_DEVICES, 24, name);
        }
        else if (file->file_attr & SIE_FS_FA_DIRECTORY) {
            item->icon = Sie_Resources_LoadIMGHDR(SIE_RESOURCES_TYPE_PLACES, 24, "folder");
        } else {
            char *ext = Sie_Ext_GetExtByFileName(file->file_name);
            if (ext) {
                item->icon = Sie_Resources_LoadIMGHDR(SIE_RESOURCES_TYPE_EXT, 24, ext);
                if (!item->icon) {
                    item->icon = Sie_Resources_LoadIMGHDR(SIE_RESOURCES_TYPE_EXT, 24, "unknown");
                }
                mfree(ext);
            } else {
                item->icon = Sie_Resources_LoadIMGHDR(SIE_RESOURCES_TYPE_EXT, 24, "unknown");
            }
        }
        // ws
        char *name = (file->alias) ? file->alias : file->file_name;
        size_t len = strlen(name);
        item->ws = AllocWS(len);
        str_2ws(item->ws, name, len);
        // color
        if (file->file_attr & SIE_FS_FA_HIDDEN) {
            item->color = color_hidden;
        }
        file = file->next;
        i++;
    }
    *count = i;
    return items;
};

static inline char GetAttr(int attr, char c) {
    return (CURRENT_FILE->file_attr & attr) ? c : '-';
}

void UpdateHeader() {
    if (CURRENT_FILE) {
        char hdr[32];
        sprintf(hdr, "%c%c%c%c\t%d/%d",
                 GetAttr(SIE_FS_FA_READONLY, 'r'),
                 GetAttr(SIE_FS_FA_HIDDEN, 'h'),
                 GetAttr(SIE_FS_FA_SYSTEM, 's'),
                 GetAttr(SIE_FS_FA_DIRECTORY, 'd'),
                 MENU->row + 1, MENU->n_items);
        Sie_GUI_Surface_SetHeader(SURFACE, hdr);
    } else {
        Sie_GUI_Surface_SetHeader(SURFACE, "");
    }
}

void SetCurrentFile(SIE_FILE *files, unsigned int id) {
    if (CURRENT_FILE) {
        Sie_FS_DestroyFileElement(CURRENT_FILE);
        CURRENT_FILE = NULL;
    }
    SIE_FILE *file = Sie_FS_GetFileByID(files, id);
    if (file) {
        CURRENT_FILE = Sie_FS_CopyFileElement(file);
    }
}

void ChangeDir(MAIN_GUI *data, const char *path) {
    Sie_FS_DestroyFiles(FILES);
    Sie_FS_DestroyFiles(SELECTED_FILES);
    FILES = NULL;
    SELECTED_FILES = NULL;
    Sie_Menu_List_Destroy(MENU);
    MENU = Sie_Menu_List_Init(SURFACE->gui_id);

    path_stack_t *p = NULL;
    if (strcmp(path, ".") == 0) { // update
        p = PATH_STACK;
    } else if (strcmp(path, "..") == 0) { // back
        p = PathStack_Pop(PATH_STACK);
        PATH_STACK = p;
    } else { // enter new dir
        p = PathStack_Add(PATH_STACK, path);
        PATH_STACK = p;
    }

    if (!strlen(p->dir_name)) { // root
        FILES = InitRootFiles();
    } else {
        char *mask = NULL;
        mask = malloc(strlen(p->dir_name) + 1 + 1);
        sprintf(mask, "%s*", p->dir_name);
        FILES = Sie_FS_FindFiles(mask);
        mfree(mask);
    }
    switch (SORT) {
        case SORT_BY_NAME_ASC:
            FILES = Sie_FS_SortFilesByNameAsc(FILES, 1);
            break;
        case SORT_BY_NAME_DESC:
            FILES = Sie_FS_SortFilesByNameDesc(FILES, 1);
            break;
        default:
            FILES = Sie_FS_SortFilesByNameAsc(FILES, 1);
            break;
    }
    if (!SHOW_HIDDEN_FILES) {
        FILES = Sie_FS_ExcludeFilesByFileAttr(FILES, SIE_FS_FA_HIDDEN);
    }
    if (FILES) {
        MENU->items = InitItems(FILES, &(MENU->n_items));
        p->row = Sie_Menu_List_SetRow(MENU, p->row);
    }
    Sie_Menu_List_Refresh(MENU);
    SetCurrentFile(FILES, MENU->row);
    UpdateHeader();
    LockSched();
    UpdateCSMname();
    UnlockSched();
}

/**********************************************************************************************************************/

static void OnRedraw(MAIN_GUI *data) {
    Sie_GUI_Surface_Draw(SURFACE);
    Sie_Menu_List_Draw(MENU);
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    ChangeDir(data, "");
    UpdateHeader();
    data->gui.state = 1;
}

static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
    Sie_FS_DestroyFiles(FILES);
    Sie_FS_DestroyFiles(SELECTED_FILES);
    if (CURRENT_FILE) {
        Sie_FS_DestroyFileElement(CURRENT_FILE);
    }
    Sie_Menu_List_Destroy(MENU);
    DestroyPathStack(PATH_STACK);
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(SURFACE);
}

static void OnUnFocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    Sie_GUI_Surface_OnUnFocus(SURFACE);
    data->gui.state = 1;
}

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    Sie_Menu_List_OnKey(MENU, msg);
    if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        WSHDR *ws;
        char path[256];
        switch (msg->gbsmsg->submess) {
            case SIE_MENU_LIST_KEY_PREV:
            case SIE_MENU_LIST_KEY_NEXT:
                PATH_STACK->row = MENU->row;
                SetCurrentFile(FILES, MENU->row);
                UpdateHeader();
                Sie_GUI_Surface_Draw(SURFACE);
                break;
            case SIE_MENU_LIST_KEY_ENTER:
                if (MENU->n_items) {
                    if (!SELECTED_FILES) {
                        SIE_FILE *file;
                        ws = MENU->items[MENU->row].ws;
                        ws_2str(ws, path, wstrlen(ws));
                        file = Sie_FS_GetFileByFileName(FILES, path);
                        if (!file) {
                            file = Sie_FS_GetFileByAlias(FILES, path);
                        }
                        if (file) {
                            if (file->file_attr & (SIE_FS_FA_VOLUME | SIE_FS_FA_DIRECTORY)) {
                                sprintf(path, "%s%s\\", PATH_STACK->dir_name, file->file_name);
                                ChangeDir(data, path);
                                Sie_GUI_Surface_Draw(SURFACE);
                                Sie_Menu_List_Draw(MENU);
                            } else {
                                size_t len;
                                sprintf(path, "%s%s", PATH_STACK->dir_name, file->file_name);
                                len = strlen(path);
                                ws = AllocWS((int) (len + 1));
                                str_2ws(ws, path, len);
                                ExecuteFile(ws, NULL, NULL);
                                FreeWS(ws);
                            }
                        }
                    }
                    else {
                        ToggleSelect();
                    }
                }
                break;
            case LEFT_SOFT:
                CreateMenuOptions();
                break;
            case RIGHT_SOFT:
                if (!strlen(PATH_STACK->dir_name)) { // root
                    return 1;
                } else {
                    ChangeDir(data, "..");
                    Sie_GUI_Surface_Draw(SURFACE);
                    Sie_Menu_List_Draw(MENU);
                }
                break;
            case GREEN_BUTTON:
                Paste();
                break;
            case '*':
                ToggleSelect();
                break;
            case '#':
                Delete();
                break;
        }
    }
    return 0;
}

static int OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    return Sie_GUI_Surface_OnKey(SURFACE, data, msg);
}

extern void kill_data(void *p, void (*func_p)(void *));

static int method8(void) { return 0; }

static int method9(void) { return 0; }

const void *const gui_methods[11] = {
        (void*)OnRedraw,
        (void*)OnCreate,
        (void*)OnClose,
        (void*)OnFocus,
        (void*)OnUnFocus,
        (void*)OnKey,
        0,
        (void*)kill_data,
        (void*)method8,
        (void*)method9,
        0
};

void CreateDefaultFiles() {
    WSHDR *ws;
    char path[256];
    unsigned int err;
    _mkdir(DIR_TEMPLATES, &err);
    ws = AllocWS(64);
    wsprintf(ws, "%s%t", DIR_TEMPLATES, "New file.txt");
    ws_2str(ws, path, 255);
    FreeWS(ws);
    if (!Sie_FS_FileExists(path)) {
        Sie_FS_CreateFile(path, &err);
    }
}

static void maincsm_oncreate(CSM_RAM *data) {
    DIR_TEMPLATES = malloc(32);
    sprintf(DIR_TEMPLATES, "%d:\\%s", DEFAULT_DISK, "Templates\\");
    Sie_SubProc_Run(CreateDefaultFiles, NULL);

    const SIE_GUI_SURFACE_HANDLERS handlers = {
            NULL,
            (int(*)(void *, GUI_MSG *msg))_OnKey,
    };
    MAIN_CSM *csm = (MAIN_CSM*)data;
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    zeromem(main_gui, sizeof(MAIN_GUI));
    Sie_GUI_InitCanvas(&canvas);
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    csm->csm.state = 0;
    csm->csm.unk1 = 0;
    csm->main_gui = main_gui;
    SURFACE = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &handlers,
                                   CreateGUI(main_gui));
    GUI_STACK = Sie_GUI_Stack_Add(NULL, &(main_gui->gui), SURFACE->gui_id);
}

void KillElf() {
    extern void *__ex;
    elfclose(&__ex);
}

static void maincsm_onclose(CSM_RAM *csm) {
    mfree(DIR_TEMPLATES);
    SUBPROC((void *)KillElf);
}

void Reload(MAIN_GUI *data) {
    ChangeDir(data, ".");
    DirectRedrawGUI_ID(SURFACE->gui_id);
}

static int maincsm_onmessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    if ((msg->msg == MSG_GUI_DESTROYED) && ((int)msg->data0 == SURFACE->gui_id)) {
        csm->csm.state = -3;
    }
    else if (msg->msg == MSG_IPC) {
        IPC_REQ *ipc = (IPC_REQ*)msg->data0;
        if (strcmp(ipc->name_to, IPC_NAME) == 0) {
            if (msg->submess == IPC_RELOAD) {
                Reload(csm->main_gui);
            } else if (msg->submess == IPC_SET_ROW_BY_FILE_NAME) {
                IPC_DATA *data = ipc->data;
                if (data->gui_id == SURFACE->gui_id) {
                    unsigned int row, err = 0;
                    const char *file_name = (const char*)data->data;
                    size_t len = strlen(file_name);
                    WSHDR *ws = AllocWS(len);
                    str_2ws(ws, file_name, len);
                    ChangeDir(csm->main_gui, ".");
                    row = Sie_Menu_List_GetRowByName_ws(MENU, ws, &err);
                    FreeWS(ws);
                    if (!err) {
                        PATH_STACK->row = Sie_Menu_List_SetRow(MENU, row);
                        Sie_Menu_List_Refresh(MENU);
                        DirectRedrawGUI_ID(SURFACE->gui_id);
                        SetCurrentFile(FILES, row);
                    }
                }
            }
        }
    }
    return 1;
}

static const struct {
    CSM_DESC maincsm;
    WSHDR maincsm_name;
} MAINCSM = {
        {
                maincsm_onmessage,
                maincsm_oncreate,
#ifdef NEWSGOLD
                0,
                0,
                0,
                0,
#endif
                maincsm_onclose,
                sizeof(MAIN_CSM),
                1,
                &minus11
        },
        {
                maincsm_name_body,
                NAMECSM_MAGIC1,
                NAMECSM_MAGIC2,
                0x0,
                139,
                0
        }
};

void UpdateCSMname() {
    WSHDR *maincsm_name_ws = (WSHDR *)&MAINCSM.maincsm_name;
    wsprintf(maincsm_name_ws, "%t", MAIN_CSM_NAME);
    size_t len = strlen(PATH_STACK->dir_name);
    if (len) {
        WSHDR *ws1 = AllocWS(8);
        WSHDR *ws2 = AllocWS(len);
        wsprintf(ws1, "%s", " - ");
        str_2ws(ws2, PATH_STACK->dir_name, len);
        wstrcat(maincsm_name_ws, ws1);
        wstrcat(maincsm_name_ws, ws2);
        FreeWS(ws1);
        FreeWS(ws2);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmain"

int main(const char *exename, const char *fname) {
    MAIN_CSM main_csm;
    sscanf(exename, "%d:\\", &DEFAULT_DISK);
    PATH_STACK = InitPathStack();
    LockSched();
    UpdateCSMname();
    CreateCSM(&MAINCSM.maincsm, &main_csm, 0);
    UnlockSched();
    return 0;
}

#pragma GCC diagnostic pop
