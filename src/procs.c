#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "ipc.h"
#include "file.h"
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

void DeleteCurrentFile() {
    CloseMenuOptionsGUI();

    unsigned int err;
    const size_t len1 = strlen(CURRENT_FILE.dir);
    const size_t len2 = strlen(CURRENT_FILE.sie_file->file_name);
    char *path = malloc(len1 + len2 + 1);
    sprintf(path, "%s%s", CURRENT_FILE.dir, CURRENT_FILE.sie_file->file_name);
    _unlink(path, &err);
    ipc_refresh();
    mfree(path);
}
