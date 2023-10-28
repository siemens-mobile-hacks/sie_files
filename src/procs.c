#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "ipc.h"
#include "path_stack.h"

extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;
extern path_stack_t *PATH_STACK;
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

void Paste() {
    static SIE_FILE *top = NULL;
    top = (COPY_FILES) ? COPY_FILES : MOVE_FILES;

    char *get_unique_path(const char *file_name) {
        const size_t len_suffix = 32;
        char *dir_name = PATH_STACK->dir_name;
        char *dest = malloc(strlen(file_name) + len_suffix + 1);
        char *dest_path = malloc(strlen(dir_name) + strlen(file_name) + len_suffix + 1);
        unsigned int i = 0;
        while (1) {
            char suffix[len_suffix], *ext;
            sprintf(suffix, "(%d)", i + 1);
            ext = strrchr(file_name, '.');
            if (ext) {
                strncpy(dest, file_name, ext - file_name);
                dest[ext - file_name] = '\0';
                strcat(dest, suffix);
                strcat(dest, ext);
            } else {
                strcpy(dest, file_name);
                strcat(dest, suffix);
            }
            sprintf(dest_path, "%s%s", dir_name, dest);
            if (!Sie_FS_FileExists(dest_path)) {
                mfree(dest);
                return dest_path;
            } else {
                i += 1;
            }
        }
    }
    void paste(int flag, void *data) {
        SIE_FILE *p = Sie_FS_DeleteFileElement(top, (SIE_FILE*)data);
        if (p) {
            char *src = Sie_FS_GetPathByFile(p);
            if (flag == SIE_GUI_MSG_BOX_CALLBACK_YES) {
                char *dest = get_unique_path(p->file_name);
                if (p->file_attr & FA_DIRECTORY) {
                    // CopyFileRecursive
                    ShowMSG(1, (int)"Is directory");
                } else {
                    Sie_FS_CopyFile(src, dest);
                }
                mfree(dest);
            }
            mfree(src);
            if (!p->prev && !p->next) {
                COPY_FILES = MOVE_FILES = NULL;
            }
            Sie_FS_DestroyFileElement(p);
            ipc_redraw();
        }
    }

    if (top) {
        char *dir_name = PATH_STACK->dir_name;
        SIE_FILE *p = top;
        while (1) {
            SIE_FILE *next = p->next;
            char *src = Sie_FS_GetPathByFile(p);
            if (strcmpi(dir_name, p->dir_name) == 0) { // current dir
                char *dest = get_unique_path(p->file_name);
                if (COPY_FILES) {
                    Sie_FS_CopyFile(src, dest);
                }
                mfree(dest);
                if (!next) {
                    ipc_redraw();
                    Sie_FS_DestroyFiles(COPY_FILES);
                    COPY_FILES = NULL;
                    break;
                }
            } else {
                char *dest = malloc(strlen(dir_name) + strlen(p->file_name) + 1);
                sprintf(dest, "%s%s", dir_name, p->file_name);
                if (Sie_FS_FileExists(dest)) {
                    SIE_GUI_MSG_BOX_CALLBACK callback;
                    callback.proc = paste;
                    callback.data = p;
                    Sie_GUI_MsgBox("Файл существует", "Вставить", "Заменить", &callback);
                    if (!next) {
                        break;
                    }
                } else {
                    unsigned int err;
                    if (COPY_FILES) {
                        Sie_FS_CopyFile(src, dest);
                        if (!next) {
                            ipc_redraw();
                            Sie_FS_DestroyFiles(COPY_FILES);
                            COPY_FILES = NULL;
                            break;
                        }
                    } else {
                        fmove(src, dest, &err);
                        if (!next) {
                            ipc_redraw();
                            Sie_FS_DestroyFiles(MOVE_FILES);
                            MOVE_FILES = NULL;
                            break;
                        }
                    }
                }
                mfree(dest);
            }
            mfree(src);
            p = next;
        }
    }
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
    ipc_redraw();
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
