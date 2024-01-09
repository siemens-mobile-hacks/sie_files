#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"

extern SIE_GUI_STACK *GUI_STACK;
extern const char *DIR_TEMPLATES;

SIE_GUI_BOX_GUI *BOX_GUI;

void SUBPROC_CreateNewFile(SIE_MENU_LIST_ITEM *menu_item) {
    size_t len = wstrlen(menu_item->ws) * 2;
    SIE_FILE *src_file = malloc(sizeof(SIE_FILE));
    zeromem(src_file, sizeof(SIE_FILE));
    src_file->file_name = malloc(len + 1);
    ws_2str(menu_item->ws, src_file->file_name, len);
    src_file->dir_name = malloc(strlen(DIR_TEMPLATES) + 1);
    strcpy(src_file->dir_name, DIR_TEMPLATES);
    SIE_FILE *dest_file = GetUniqueFileInCurrentDir(src_file);

    char *src = Sie_FS_GetPathByFile(src_file);
    char *dest = Sie_FS_GetPathByFile(dest_file);

    Sie_FS_CopyFile(src, dest);
    mfree(src);
    mfree(dest);
    IPC_CloseChildrenGUI(0);

    WSHDR *ws = AllocWS(len);
    str_2ws(ws, dest_file->file_name, len);
    Sie_FS_DestroyFileElement(src_file);
    Sie_FS_DestroyFileElement(dest_file);
    IPC_SetRowByFileName_ws(ws);

    BOX_GUI = NULL;
}

void CreateNewFile(SIE_MENU_LIST_ITEM *menu_item, unsigned int row) {
    BOX_GUI = Sie_GUI_MsgBox("Создание файла...");
    SUBPROC(SUBPROC_CreateNewFile, menu_item);
    GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(BOX_GUI->gui), BOX_GUI->gui_id);
}
