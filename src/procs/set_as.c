#include <swilib.h>
#include <nu_swilib.h>
#include <sie/sie.h>
#include "../helpers.h"

extern SIE_FILE *CURRENT_FILE;
extern unsigned int MAIN_GUI_ID;

static SIE_GUI_BOX *BOX_GUI;

void SetWallpaper_Proc() {
    Sie_GUI_CloseGUI_GBS(BOX_GUI->surface->gui_id);
}

static void Proc() {
    const size_t len1 = strlen(CURRENT_FILE->dir_name);
    const size_t len2 = strlen(CURRENT_FILE->file_name);
    WSHDR *ws = AllocWS((int)(len1 + len2 + 1));
    wsprintf(ws, "%s%s", CURRENT_FILE->dir_name, CURRENT_FILE->file_name);
    BOX_GUI = Sie_GUI_WaitBox();
    Sie_Resources_SetWallpaper(ws, SetWallpaper_Proc);
    FreeWS(ws);
}

void SetAsWallpaper() {
    static GBSTMR tmr;
    static SIE_GUI_FOCUS_DATA data;
    data.gui_id = MAIN_GUI_ID;
    data.proc = Proc;
    data.data = NULL;
    CloseChildrenGUI();
    Sie_GUI_FocusGUI(&tmr, &data);
}
