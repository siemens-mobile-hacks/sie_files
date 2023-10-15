#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "ipc.h"
#include "menu_set_as.h"
#include "menu_options.h"
#include "menu_new_file.h"

extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *COPY_FILE;
extern unsigned int MAIN_GUI_ID;
extern SIE_GUI_STACK *GUI_STACK;

void CreateDiskInfoGUI() {
    ShowMSG(1, (int)"Create disk info gui");
}

void CreateFile() {
    ShowMSG(1, (int)"Create file");
}

void CreateDir() {
    ShowMSG(1, (int)"Create directory");
}

void CopyFile() {
    COPY_FILE = Sie_FS_CopyFileElement(CURRENT_FILE);
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}

void Paste() {
    char *dir_name = CURRENT_FILE->dir_name;
    size_t len_dir_name = strlen(dir_name);

    if (COPY_FILE) {
        SIE_FILE *p = COPY_FILE;
        unsigned int i_exists = 0;
        while (p) {
            char *dest_file_name = malloc(strlen(p->file_name) + 32 + 1);
            if (!i_exists) {
                strcpy(dest_file_name, p->file_name);
            } else {
                char s[8], *ext;
                sprintf(s, "(%d)", i_exists);
                ext = strrchr(p->file_name, '.');
                if (ext) {
                    strncpy(dest_file_name, p->file_name, ext - p->file_name);
                    dest_file_name[ext - p->file_name] = '\0';
                    strcat(dest_file_name, s);
                    strcat(dest_file_name, ext);
                } else {
                    strcpy(dest_file_name, p->file_name);
                    strcat(dest_file_name, s);
                }
            }

            char *src = Sie_FS_GetPathByFile(p);
            char *dest = malloc(len_dir_name + strlen(dest_file_name) + 1);
            sprintf(dest, "%s%s", CURRENT_FILE->dir_name, dest_file_name);
            if (Sie_FS_FileExists(dest)) {
                i_exists += 1;
                mfree(src);
                mfree(dest);
                continue;
            } else {
                Sie_FS_CopyFile(dest, src);
                mfree(src);
                mfree(dest);
                i_exists = 0;
            }
            p = p->next;
        }
        Sie_FS_DestroyFiles(COPY_FILE);
        COPY_FILE = NULL;
    }

    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
    ipc_redraw();
}

/**********************************************************************************************************************/

void SetAs() {
    CreateMenuSetAsGUI();
}

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
        if (p->file_attr & FA_DIRECTORY) {
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
