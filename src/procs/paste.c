#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../path_stack.h"

extern SIE_FILE *COPY_FILES, *MOVE_FILES;
extern path_stack_t *PATH_STACK;
extern unsigned int MAIN_GUI_ID;
extern SIE_GUI_STACK *GUI_STACK;

char *GetUniquePath(const char *file_name) {
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

void MsgProc(int flag, void *data) {
    SIE_FILE *top = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    SIE_FILE *p = Sie_FS_DeleteFileElement(top, (SIE_FILE*)data);
    if (p) {
        char *src = Sie_FS_GetPathByFile(p);
        if (flag == SIE_GUI_MSG_BOX_CALLBACK_YES) {
            char *dest = GetUniquePath(p->file_name);
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
        IPC_Redraw();
    }
}

void Paste() {
    SIE_FILE *top = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    if (top) {
        char *dir_name = PATH_STACK->dir_name;
        SIE_FILE *p = top;
        while (1) {
            SIE_FILE *next = p->next;
            char *src = Sie_FS_GetPathByFile(p);
            if (strcmpi(dir_name, p->dir_name) == 0) { // current dir
                char *dest = GetUniquePath(p->file_name);
                if (COPY_FILES) {
                    Sie_FS_CopyFile(src, dest);
                }
                mfree(dest);
                if (!next) {
                    IPC_Redraw();
                    Sie_FS_DestroyFiles(COPY_FILES);
                    COPY_FILES = NULL;
                    break;
                }
            } else {
                char *dest = malloc(strlen(dir_name) + strlen(p->file_name) + 1);
                sprintf(dest, "%s%s", dir_name, p->file_name);
                if (Sie_FS_FileExists(dest)) {
                    SIE_GUI_MSG_BOX_CALLBACK callback;
                    callback.proc = MsgProc;
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
                            IPC_Redraw();
                            Sie_FS_DestroyFiles(COPY_FILES);
                            COPY_FILES = NULL;
                            break;
                        }
                    } else {
                        fmove(src, dest, &err);
                        if (!next) {
                            IPC_Redraw();
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
    IPC_Redraw();
}
