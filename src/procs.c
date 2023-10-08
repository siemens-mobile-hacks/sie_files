#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "ipc.h"
#include "files.h"
#include "menu_set_as.h"
#include "menu_options.h"

extern file_t CURRENT_FILE;

void CreateDiskInfoGUI() {
    ShowMSG(1, (int)"Create disk info gui");
}

void CreateDir() {
    ShowMSG(1, (int)"Create directory");
}

/**********************************************************************************************************************/

void SetAs() {
    CreateMenuSetAsGUI();
}

void SetAsWallpaper() {
    const size_t len1 = strlen(CURRENT_FILE.dir);
    const size_t len2 = strlen(CURRENT_FILE.sie_file->file_name);
    WSHDR *ws = AllocWS((int)(len1 + len2 + 1));
    wsprintf(ws, "%s%s", CURRENT_FILE.dir, CURRENT_FILE.sie_file->file_name);
    Sie_Resources_SetWallpaper(ws);
    FreeWS(ws);
    CloseMenuOptionsGUI();
    CloseMenuSetAsGUI();
}

/**********************************************************************************************************************/

void DeleteFiles(const files_list_t *files) {
    unsigned int err;
    files_list_t *p = (files_list_t*)files;
    while (p != NULL) {
        const file_t *file = p->file;
        const size_t len1 = strlen(file->dir);
        const size_t len2 = strlen(file->sie_file->file_name);
        char *path = malloc(len1 + len2 + 1);
        sprintf(path, "%s%s", file->dir, file->sie_file->file_name);
        _unlink(path, &err);
        mfree(path);
        p = p->next;
    }
    ipc_redraw();

    CloseMenuOptionsGUI();
}
