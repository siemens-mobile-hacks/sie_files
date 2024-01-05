#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "ipc.h"
#include "path_stack.h"
#include "menu_options.h"
#include "procs/procs.h"

#define MAIN_CSM_NAME "Файлы"

typedef struct {
    GUI gui;
    SIE_FILE *files;
    SIE_GUI_SURFACE *surface;
} MAIN_GUI;

typedef struct {
    CSM_RAM csm;
    MAIN_GUI *main_gui;
    int gui_id;
} MAIN_CSM;


static int _OnKey(MAIN_GUI *data, GUI_MSG *msg);

/**********************************************************************************************************************/
int DEFAULT_DISK;
char *DIR_TEMPLATES;

const int minus11 = -11;
unsigned short maincsm_name_body[140];
RECT canvas = {0, 0, 0, 0};

SIE_MENU_LIST *MENU;
SIE_FILE *CURRENT_FILE;
SIE_FILE *COPY_FILES, *MOVE_FILES;
path_stack_t *PATH_STACK;
unsigned int MAIN_GUI_ID;
SIE_GUI_STACK *GUI_STACK;

unsigned int SHOW_HIDDEN_FILES = 1;

const char *DIR_ROOT = "0:\\zbin\\usr\\sie_files\\";
const char *DIR_IMG = "0:\\zbin\\usr\\sie_files\\img\\";

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
    SIE_FILE *p = current;
    while (true) {
        if (!(p->prev)) {
            break;
        }
        p = p->prev;
    }
    CURRENT_FILE = p;
    return p;
}

SIE_MENU_LIST_ITEM *InitItems(SIE_FILE *top, unsigned int *count) {
    static char color_hidden[] = COLOR_TEXT_DISABLED;

    SIE_MENU_LIST_ITEM *items = NULL;

    SIE_FILE *file = top;
    unsigned int i = 0;
    while (file) {
        if (!SHOW_HIDDEN_FILES && file->file_attr & SIE_FS_FA_HIDDEN) { // pass hidden files
            file = file->next;
            continue;
        }

        SIE_MENU_LIST_ITEM *item;
        items = realloc(items, sizeof(SIE_MENU_LIST_ITEM) * (i + 1));
        item = &(items[i]);
        zeromem(item, sizeof(SIE_MENU_LIST_ITEM));

        // icon
        SIE_RESOURCES_IMG *res_img = NULL;
        if (file->file_attr & SIE_FS_FA_VOLUME) {
            char name[8];
            if (strcmp(file->file_name, "4:") == 0) {
                strcpy(name, "mmcard");
            } else {
                strcpy(name, "disk");
            }
            res_img = Sie_Resources_LoadImage(SIE_RESOURCES_TYPE_DEVICES, 24, name);
        }
        else if (file->file_attr & SIE_FS_FA_DIRECTORY) {
            res_img = Sie_Resources_LoadImage(SIE_RESOURCES_TYPE_PLACES, 24, "folder");
        } else {
            char *ext = Sie_Ext_GetExtByFileName(file->file_name);
            if (ext) {
                res_img = Sie_Resources_LoadImage(SIE_RESOURCES_TYPE_EXT, 24, ext);
                if (!res_img) {
                    res_img = Sie_Resources_LoadImage(SIE_RESOURCES_TYPE_EXT, 24, "unknown");
                }
                mfree(ext);
            } else {
                res_img = Sie_Resources_LoadImage(SIE_RESOURCES_TYPE_EXT, 24, "unknown");
            }
        }
        if (res_img) {
            item->icon = res_img->icon;
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

inline char GetAttr(int attr, char c) {
    return (CURRENT_FILE->file_attr & attr) ? c : '-';
}

void UpdateHeader(MAIN_GUI *data) {
    if (CURRENT_FILE) {
        wsprintf(data->surface->hdr_ws, "%c%c%c%c\t%d/%d",
                 GetAttr(SIE_FS_FA_READONLY, 'r'),
                 GetAttr(SIE_FS_FA_HIDDEN, 'h'),
                 GetAttr(SIE_FS_FA_SYSTEM, 's'),
                 GetAttr(SIE_FS_FA_DIRECTORY, 'd'),
                 MENU->row + 1, MENU->n_items);
    } else {
        wsprintf(data->surface->hdr_ws, "");
    }
}

void ChangeDir(MAIN_GUI *data, const char *path) {
    Sie_FS_DestroyFiles(data->files);
    data->files = NULL;
    Sie_Menu_List_Destroy(MENU);
    MENU = Sie_Menu_List_Init(MAIN_GUI_ID);

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
        data->files = InitRootFiles();
        MENU->items = InitItems(data->files, &(MENU->n_items));
        MENU->row = p->row;
    } else {
        char *mask = NULL;
        mask = malloc(strlen(p->dir_name) + 1 + 1);
        sprintf(mask, "%s*", p->dir_name);
        data->files = Sie_FS_FindFiles(mask);
        data->files = Sie_FS_SortFilesByName(data->files, 1);
        if (data->files) {
            MENU->items = InitItems(data->files, &(MENU->n_items));
            if (p->row >= MENU->n_items) {
                p->row = MENU->n_items - 1;
            }
            MENU->row = p->row;
            CURRENT_FILE = Sie_FS_GetFileByID(data->files, MENU->row);
        } else {
            CURRENT_FILE = NULL;
        }
        Sie_Menu_List_Refresh(MENU);
        mfree(mask);
    }
    if (MENU->row >= MENU->n_items) {
        MENU->row = 0;
    }
    UpdateHeader(data);
}

/**********************************************************************************************************************/

static void OnRedraw(MAIN_GUI *data) {
    Sie_GUI_Surface_Draw(data->surface);
    Sie_Menu_List_Draw(MENU);
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    Sie_FT_Init();
    Sie_Resources_Init();
    PATH_STACK = InitPathStack();

    const SIE_GUI_SURFACE_HANDLERS handlers = {
            NULL,
            (int(*)(void *, GUI_MSG *msg))_OnKey,
    };
    data->surface = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &handlers);
    data->files = InitRootFiles();
    ChangeDir(data, "");
    UpdateHeader(data);

    data->gui.state = 1;
}

static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
    Sie_FS_DestroyFiles(data->files);
    Sie_Menu_List_Destroy(MENU);
    DestroyPathStack(PATH_STACK);
    Sie_FT_Destroy();
    Sie_Resources_Destroy();
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(data->surface);
}

static void OnUnFocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    Sie_GUI_Surface_OnUnFocus(data->surface);
    data->gui.state = 1;
}

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    char path[1024];

    Sie_Menu_List_OnKey(MENU, msg);

    if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        switch (msg->gbsmsg->submess) {
            case SIE_MENU_LIST_KEY_PREV:
            case SIE_MENU_LIST_KEY_NEXT:
                PATH_STACK->row = MENU->row;
                CURRENT_FILE = Sie_FS_GetFileByID(data->files, MENU->row);
                UpdateHeader(data);
                Sie_GUI_Surface_Draw(data->surface);
                break;
            case SIE_MENU_LIST_KEY_ENTER:
                if (MENU->n_items) {
                    SIE_FILE *file;
                    WSHDR *ws = MENU->items[MENU->row].ws;
                    ws_2str(ws, path, wstrlen(ws));
                    file = Sie_FS_GetFileByFileName(data->files, path);
                    if (!file) {
                        file = Sie_FS_GetFileByAlias(data->files, path);
                    }
                    if (file) {
                        if (file->file_attr & SIE_FS_FA_VOLUME || file->file_attr & SIE_FS_FA_DIRECTORY) {
                            sprintf(path, "%s%s\\", PATH_STACK->dir_name, file->file_name);
                            //data->path_list_last->row = data->menu->row;
                            ChangeDir(data, path);
                            Sie_GUI_Surface_Draw(data->surface);
                            Sie_Menu_List_Draw(MENU);
                        } else {
                            WSHDR *ws = AllocWS(12);
                            size_t len;
                            sprintf(path, "%s%s", PATH_STACK->dir_name, file->file_name);
                            len = strlen(path);
                            ws = AllocWS((int)(len + 1));
                            str_2ws(ws, path, len);
                            ExecuteFile(ws, NULL, NULL);
                            FreeWS(ws);
                        }
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
                    Sie_GUI_Surface_Draw(data->surface);
                    Sie_Menu_List_Draw(MENU);
                }
                break;
            case '#':
                if (CURRENT_FILE) {
                    Delete();
                }
                break;
        }
    }
    return 0;
}

static int OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    return Sie_GUI_Surface_OnKey(data->surface, data, msg);
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
    wsprintf(ws, "%s%t", DIR_TEMPLATES, "Текстовый файл.txt");
    ws_2str(ws, path, 255);
    FreeWS(ws);
    if (!Sie_FS_FileExists(path)) {
        Sie_FS_CreateFile(path);
    }
}

static void maincsm_oncreate(CSM_RAM *data) {
    DIR_TEMPLATES = malloc(32);
    sprintf(DIR_TEMPLATES, "%d:\\%s", DEFAULT_DISK, "Templates\\");
    SUBPROC((void*)CreateDefaultFiles);

    MAIN_CSM *csm = (MAIN_CSM*)data;
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    zeromem(main_gui, sizeof(MAIN_GUI));
    Sie_GUI_InitCanvas(&canvas);
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    csm->csm.state = 0;
    csm->csm.unk1 = 0;
    MAIN_GUI_ID = csm->gui_id = CreateGUI(main_gui);
    csm->main_gui = main_gui;
    GUI_STACK = Sie_GUI_Stack_Add(NULL, &(main_gui->gui), csm->gui_id);
}

void KillElf() {
    extern void *__ex;
    elfclose(&__ex);
}

static void maincsm_onclose(CSM_RAM *csm) {
    mfree(DIR_TEMPLATES);
    SUBPROC((void *)KillElf);
}

static int maincsm_onmessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    if ((msg->msg == MSG_GUI_DESTROYED) && ((int)msg->data0 == csm->gui_id)) {
        csm->csm.state = -3;
    }
    else if (msg->msg == MSG_IPC) {
        IPC_REQ *ipc = (IPC_REQ*)msg->data0;
        if (strcmp(ipc->name_to, IPC_NAME) == 0) {
            if (msg->submess == IPC_REDRAW) {
                ChangeDir(csm->main_gui, ".");
                DirectRedrawGUI();
            } else if (msg->submess == IPC_SET_ROW_BY_FILE_NAME_WS) {
                unsigned int row = 0, err = 0;
                ChangeDir(csm->main_gui, ".");
                row = Sie_Menu_List_GetIdByName_ws(MENU, ipc->data, &err);
                if (!err) {
                    Sie_Menu_List_SetRow(MENU, row);
                    Sie_Menu_List_Refresh(MENU);
                    DirectRedrawGUI();
                } else {
                    ShowMSG(1, (int)err);
                }
                CURRENT_FILE = Sie_FS_GetFileByID(csm->main_gui->files, row);
                FreeWS(ipc->data);
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

void UpdateCSMname(void) {
    wsprintf((WSHDR *)&MAINCSM.maincsm_name, "%t", MAIN_CSM_NAME);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmain"

int main(const char *exename, const char *fname) {
    MAIN_CSM main_csm;
    sscanf(exename, "%d:\\", &DEFAULT_DISK);
    LockSched();
    UpdateCSMname();
    CreateCSM(&MAINCSM.maincsm, &main_csm, 0);
    UnlockSched();
    return 0;
}

#pragma GCC diagnostic pop
