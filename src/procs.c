#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "ipc.h"
#include "files.h"
#include "menu_set_as.h"
#include "menu_options.h"
#include "menu_new_file.h"

extern file_t CURRENT_FILE;
extern SIE_GUI_STACK *GUI_STACK;
extern unsigned int MAIN_GUI_ID;

void CreateDiskInfoGUI() {
    ShowMSG(1, (int)"Create disk info gui");
}

void CreateFile() {
    ShowMSG(1, (int)"Create file");
}

void CreateDir() {
    ShowMSG(1, (int)"Create directory");
}

/**********************************************************************************************************************/

void SetAs() {
    CreateMenuSetAsGUI();
}

void SetAsWallpaper() {
    const size_t len1 = strlen(CURRENT_FILE.sie_file->dir_name);
    const size_t len2 = strlen(CURRENT_FILE.sie_file->file_name);
    WSHDR *ws = AllocWS((int)(len1 + len2 + 1));
    wsprintf(ws, "%s%s", CURRENT_FILE.sie_file->dir_name, CURRENT_FILE.sie_file->file_name);
    Sie_Resources_SetWallpaper(ws);
    FreeWS(ws);
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}

/**********************************************************************************************************************/

void DeleteFiles(const files_list_t *files) {
    unsigned int err;
    files_list_t *p = (files_list_t*)files;
    while (p != NULL) {
        const file_t *file = p->file;
        char *path = Sie_FS_GetPathByFile(file->sie_file);
        if (file->sie_file->file_attr & FA_DIRECTORY) {
            Sie_FS_RemoveDirRecursive(path);
        } else {
            _unlink(path, &err);
        }
        mfree(path);
        p = p->next;
    }
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
    ipc_redraw();
}
