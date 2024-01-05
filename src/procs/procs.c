#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"
#include "../path_stack.h"

extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;
extern path_stack_t *PATH_STACK;
extern unsigned int MAIN_GUI_ID;
extern SIE_GUI_STACK *GUI_STACK;
extern const char *DIR_TEMPLATES;

void CreateDiskInfoGUI() {
    ShowMSG(1, (int)"Create disk info gui");
}

void CreateFile(SIE_MENU_LIST_ITEM *menu_item, unsigned int row) {
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

    WSHDR *ws = AllocWS(len);
    str_2ws(ws, dest_file->file_name, len);
    IPC_SetRowByFileName_ws(ws);
    Sie_FS_DestroyFileElement(src_file);
    Sie_FS_DestroyFileElement(dest_file);

    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}

void CreateDir() {
    ShowMSG(1, (int)"Create directory");
}

void CopyFile() {
    if (COPY_FILES) {
        Sie_FS_DestroyFiles(COPY_FILES);
    }
    COPY_FILES = Sie_FS_CopyFileElement(CURRENT_FILE);
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}

void MoveFile() {
    if (MOVE_FILES) {
        Sie_FS_DestroyFiles(MOVE_FILES);
    }
    MOVE_FILES = Sie_FS_CopyFileElement(CURRENT_FILE);
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}



/**********************************************************************************************************************/

void SetAsWallpaper() {
    const size_t len1 = strlen(CURRENT_FILE->dir_name);
    const size_t len2 = strlen(CURRENT_FILE->file_name);
    WSHDR *ws = AllocWS((int)(len1 + len2 + 1));
    wsprintf(ws, "%s%s", CURRENT_FILE->dir_name, CURRENT_FILE->file_name);
    Sie_Resources_SetWallpaper(ws);
    FreeWS(ws);
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}

/**********************************************************************************************************************/

void DeleteFiles(const SIE_FILE *files) {
    unsigned int err;
    SIE_FILE *p = (SIE_FILE*)files;
    while (p != NULL) {
        char *path = Sie_FS_GetPathByFile(p);
        if (p->file_attr & SIE_FS_FA_DIRECTORY) {
            Sie_FS_RemoveDirRecursive(path);
        } else {
            _unlink(path, &err);
        }
        mfree(path);
        p = p->next;
    }
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
    IPC_Redraw();
}
