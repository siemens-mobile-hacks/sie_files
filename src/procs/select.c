#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"

extern SIE_MENU_LIST *MENU;
extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *SELECTED_FILES;

void Select() {
    if (CURRENT_FILE && !(CURRENT_FILE->file_attr & SIE_FS_FA_VOLUME)) {
        SIE_MENU_LIST_ITEM *item = Sie_Menu_List_GetCurrentItem(MENU);
        Sie_Menu_List_SetItemType(item, SIE_MENU_LIST_ITEM_TYPE_CHECKBOX, 1);
        SIE_FILE *file = Sie_FS_CopyFileElement(CURRENT_FILE);
        if (!SELECTED_FILES) {
            SELECTED_FILES = file;
        } else {
            SIE_FILE *last = Sie_FS_GetLastFile(SELECTED_FILES);
            last->next = file;
            file->prev = last;
        }
        IPC_CloseChildrenGUI(0);
        DirectRedrawGUI();
    }
}

void UnSelect() {
    if (CURRENT_FILE) {
        SIE_MENU_LIST_ITEM *item = Sie_Menu_List_GetCurrentItem(MENU);
        Sie_Menu_List_SetItemType(item, SIE_MENU_LIST_ITEM_TYPE_CHECKBOX, 0);
        SIE_FILE *file = Sie_FS_DeleteFileElement(SELECTED_FILES, CURRENT_FILE);
        if (file) {
            SIE_FILE *next = file->next;
            Sie_FS_DestroyFileElement(file);

            if (file == SELECTED_FILES) { // first
                if (next) {
                    next->prev = NULL;
                    SELECTED_FILES = next;
                } else {
                    SELECTED_FILES = NULL;
                }
            }
            if (SELECTED_FILES) {
                IPC_CloseChildrenGUI(0);
                DirectRedrawGUI();
            } else {
                IPC_CloseChildrenGUI(1);
            }
        }
    }
}

void UnSelectAll() {
    Sie_FS_DestroyFiles(SELECTED_FILES);
    SELECTED_FILES = NULL;
    IPC_CloseChildrenGUI(1);
}

unsigned int IsSelectedCurrentFile() {
    return Sie_FS_ContainsFile(SELECTED_FILES, CURRENT_FILE);
}

void ToggleSelect() {
    if (IsSelectedCurrentFile()) {
        UnSelect();
    } else {
        Select();
    }
}
