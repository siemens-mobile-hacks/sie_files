#include <swilib.h>
#include <sie/sie.h>
#include "../helpers.h"

extern SIE_FILE *CURRENT_FILE;

void SetAsWallpaper() {
    const size_t len1 = strlen(CURRENT_FILE->dir_name);
    const size_t len2 = strlen(CURRENT_FILE->file_name);
    WSHDR *ws = AllocWS((int)(len1 + len2 + 1));
    wsprintf(ws, "%s%s", CURRENT_FILE->dir_name, CURRENT_FILE->file_name);
    CloseChildrenGUI();
    Sie_Resources_SetWallpaper(ws);
    FreeWS(ws);
}
