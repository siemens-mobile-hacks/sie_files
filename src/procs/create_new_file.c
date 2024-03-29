#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../msg.h"
#include "../helpers.h"

extern SIE_GUI_SURFACE *SURFACE;
extern SIE_GUI_STACK *GUI_STACK;
extern const char *DIR_TEMPLATES;

static SIE_GUI_BOX *BOX_GUI;

void SUBPROC_CreateNewFile(WSHDR *ws) {
    size_t len = wstrlen(ws) * 4;
    SIE_FILE *src_file = malloc(sizeof(SIE_FILE));
    zeromem(src_file, sizeof(SIE_FILE));
    src_file->file_name = malloc(len + 1);
    ws_2str(ws, src_file->file_name, len);
    src_file->dir_name = malloc(strlen(DIR_TEMPLATES) + 1);
    strcpy(src_file->dir_name, DIR_TEMPLATES);
    SIE_FILE *dest_file = GetUniqueFileInCurrentDir(src_file);
    FreeWS(ws);

    unsigned int err = 0;
    char *src = Sie_FS_GetPathByFile(src_file);
    char *dest = Sie_FS_GetPathByFile(dest_file);
    int copy = Sie_FS_CopyFile(src, dest, &err);
    mfree(src);
    mfree(dest);

    Sie_GUI_BoxClose(BOX_GUI);

    if (copy >= 0) {
        static char file_name[320];
        strcpy(file_name, dest_file->file_name);
        IPC_SetRowByFileName(file_name);
    } else {
        MsgBoxError_FileAction(dest_file, "create");
    }
    Sie_FS_DestroyFileElement(src_file);
    Sie_FS_DestroyFileElement(dest_file);

    BOX_GUI = NULL;
}

static void Proc(WSHDR *ws) {
    Sie_GUI_Surface_TakeScrot(SURFACE);
    BOX_GUI = Sie_GUI_WaitBox("Creating file...", SURFACE->scrot);
    Sie_SubProc_Run(SUBPROC_CreateNewFile, ws);
}

void CreateNewFile(SIE_MENU_LIST_ITEM *menu_item, unsigned int row) {
    static GBSTMR tmr;
    static SIE_GUI_FOCUS_DATA data;
    size_t len = wstrlen(menu_item->ws);
    data.gui_id = SURFACE->gui_id;
    data.proc = (void*)(void*)Proc;
    data.data = AllocWS(len);
    wstrcpy(data.data, menu_item->ws);
    CloseChildrenGUI();
    Sie_GUI_FocusGUI(&tmr, &data);
}
