#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "files.h"
#include "menu.h"
#include "procs.h"
#include "menu_new_file.h"

typedef struct {
    GUI gui;
    unsigned int gui_id;
    SIE_MENU_LIST *menu;
    SIE_GUI_SURFACE *surface;
} MAIN_GUI;

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg);

/**********************************************************************************************************************/

extern RECT canvas;
extern SIE_GUI_STACK *GUI_STACK;

extern file_t CURRENT_FILE;
extern const char *DIR_TEMPLATES;

void Delete(void) {
    void callback(int flag) {
        if (flag == SIE_GUI_MSG_BOX_CALLBACK_YES) {
            files_list_t *files = InitFilesListFromCurrentFile();
            DeleteFiles(files);
            DestroyFilesList(files);
        }
    }
    WSHDR *ws = AllocWS(32);
    wsprintf(ws, "%t", "Удалить?");
    Sie_GUI_MsgBoxYesNo(ws, callback);
    FreeWS(ws);
}

/**********************************************************************************************************************/

static void OnRedraw(MAIN_GUI *data) {
    Sie_GUI_Surface_Draw(data->surface);
    Sie_Menu_List_Draw(data->menu);
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    data->gui.state = 1;

    char **names = NULL;
    void (**procs)(void) = NULL;
    unsigned int count = 0;

    if (!strlen(CURRENT_FILE.sie_file->dir_name)) { // диски
        M_AddMenuItem("Информация о диске", CreateDiskInfoGUI);
    } else if (CURRENT_FILE.sie_file) { // каталог или файл
        char mask[64];
        sprintf(mask, "%s*", DIR_TEMPLATES);
        SIE_FILE *templates = Sie_FS_FindFiles(mask);
        M_AddMenuItem("Новый каталог", CreateDir);
        if (templates) {
            M_AddMenuItem("Новый файл", CreateMenuNewFileGUI);
            Sie_FS_DestroyFiles(templates);
        }
        if (!(CURRENT_FILE.sie_file->file_attr & FA_DIRECTORY)) { // файл
            char *ext = Sie_Strings_GetExtByFileName(CURRENT_FILE.sie_file->file_name);
            if (ext) {
                if (strcmpi(ext, "png") == 0) {
                    M_AddMenuItem("Задать как...", SetAs);
                }
                mfree(ext);
            }
        }
        else { // каталог

        }
        M_AddMenuItem("Удалить", Delete);
    } else { // пустота :-)
        M_AddMenuItem("Создать папку", CreateDir);
    }
    data->menu = M_InitMenu();
    M_DestroyMenuItems();

    const SIE_GUI_SURFACE_HANDLERS handlers = {
            NULL,
            (int(*)(void *, GUI_MSG *msg))_OnKey,
    };
    data->surface = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &handlers);
    wsprintf(data->surface->hdr_ws, "%t", "Опции");
}

static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
    DestroyMenu(data->menu);
    Sie_GUI_Surface_Destroy(data->surface);
    GUI_STACK = Sie_GUI_Stack_Pop(GUI_STACK, data->gui_id);
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(data->surface);
}

static void OnUnfocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    data->gui.state = 1;
    Sie_GUI_Surface_OnUnfocus(data->surface);
}

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    Sie_Menu_List_OnKey(data->menu, msg);
    if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
        switch (msg->gbsmsg->submess) {
            case RIGHT_SOFT:
                return 1;
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

static const void *const gui_methods[11] = {
        (void*)OnRedraw,
        (void*)OnCreate,
        (void*)OnClose,
        (void*)OnFocus,
        (void*)OnUnfocus,
        (void*)OnKey,
        0,
        (void*)kill_data,
        (void*)method8,
        (void*)method9,
        0
};

void CreateMenuOptionsGUI() {
    LockSched();
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    zeromem(main_gui, sizeof(MAIN_GUI));
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    main_gui->gui_id = CreateGUI(main_gui);
    GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(main_gui->gui), main_gui->gui_id);
    UnlockSched();
}
