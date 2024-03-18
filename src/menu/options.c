#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <sie/sie.h>
#include "view.h"
#include "create.h"
#include "set_as.h"
#include "settings.h"
#include "operations.h"
#include "../path_stack.h"
#include "../procs/procs.h"

typedef struct {
    GUI gui;
    unsigned int gui_id;
    SIE_MENU_LIST *menu;
    SIE_GUI_SURFACE *surface;
} MAIN_GUI;

static int _OnKey(MAIN_GUI *data, GUI_MSG *msg);

/**********************************************************************************************************************/

extern RECT canvas;
extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *SELECTED_FILES;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;
extern path_stack_t *PATH_STACK;
extern SIE_GUI_STACK *GUI_STACK;

/**********************************************************************************************************************/

static void OnRedraw(MAIN_GUI *data) {
    Sie_GUI_Surface_Draw(data->surface);
    Sie_Menu_List_Draw(data->menu);
}

void AddSelectItems(SIE_MENU_LIST *menu, SIE_MENU_LIST_ITEM *item) {
    if (!IsSelectedCurrentFile()) {
        item->proc = Select;
        Sie_Menu_List_AddItem(menu, item, "Select");
    }
    item->proc = SelectAll;
    Sie_Menu_List_AddItem(menu, item, "Select all");
}

static inline void AddMenuOperations(SIE_MENU_LIST *menu, SIE_MENU_LIST_ITEM *item) {
    item->proc = CreateMenuOperations;
    Sie_Menu_List_AddItem(menu, item, "Operations");
}

static inline void AddMenuView(SIE_MENU_LIST *menu, SIE_MENU_LIST_ITEM *item) {
    item->proc = CreateMenuView;
    Sie_Menu_List_AddItem(menu, item, "View");
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    SIE_MENU_LIST_ITEM item;
    zeromem(&item, sizeof(SIE_MENU_LIST_ITEM));
    data->menu = Sie_Menu_List_Init(data->gui_id);
    if (!strlen(PATH_STACK->dir_name)) { // disks
        item.proc = CreateDiskInfoGUI;
        Sie_Menu_List_AddItem(data->menu, &item, "Disk info");
        AddMenuView(data->menu, &item);
    } else if (CURRENT_FILE) { // dir or file
        if (!SELECTED_FILES) {
            item.proc = CreateMenuCreate;
            Sie_Menu_List_AddItem(data->menu, &item, "Create");
            AddSelectItems(data->menu, &item);
            if (!(CURRENT_FILE->file_attr & SIE_FS_FA_DIRECTORY)) { // file
                AddMenuOperations(data->menu, &item);
                int uid = Sie_Ext_GetExtUidByFileName(CURRENT_FILE->file_name);
                if (uid) {
                    if (uid == SIE_EXT_UID_JPG || uid == SIE_EXT_UID_PNG) {
                        item.proc = CreateMenuSetAs;
                        Sie_Menu_List_AddItem(data->menu, &item, "Set as...");
                    }
                }
            } else { // dir
                AddMenuOperations(data->menu, &item);
            }
            AddMenuView(data->menu, &item);
        } else {
            AddSelectItems(data->menu, &item);
            item.proc = UnSelectAll;
            Sie_Menu_List_AddItem(data->menu, &item, "Unselect all");
            AddMenuOperations(data->menu, &item);
        }
    } else { // empty :-)
        item.proc = CreateMenuCreate;
        Sie_Menu_List_AddItem(data->menu, &item, "Create");
        if (IsPasteAllow()) {
            AddMenuOperations(data->menu, &item);
        }
        AddMenuView(data->menu, &item);
    }
    item.proc = CreateMenuSettings;
    Sie_Menu_List_AddItem(data->menu, &item, "Settings");
    data->gui.state = 1;
}

static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
    Sie_Menu_List_Destroy(data->menu);
    Sie_GUI_Surface_Destroy(data->surface);
    GUI_STACK = Sie_GUI_Stack_Delete(GUI_STACK, data->gui_id);
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(data->surface);
}

static void OnUnFocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    data->gui.state = 1;
    Sie_GUI_Surface_OnUnFocus(data->surface);
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
        (void*)OnUnFocus,
        (void*)OnKey,
        0,
        (void*)kill_data,
        (void*)method8,
        (void*)method9,
        0
};

void CreateMenuOptions() {
    const SIE_GUI_SURFACE_HANDLERS handlers = {
            NULL,
            (int(*)(void *, GUI_MSG *msg))_OnKey,
    };
    LockSched();
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    zeromem(main_gui, sizeof(MAIN_GUI));
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    main_gui->surface = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &handlers,
                                             CreateGUI(main_gui));
    Sie_GUI_Surface_SetHeader(main_gui->surface, "Options");
    GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(main_gui->gui), main_gui->surface->gui_id);
    UnlockSched();
}
