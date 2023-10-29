#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../path_stack.h"

extern SIE_FILE *COPY_FILES, *MOVE_FILES;
extern path_stack_t *PATH_STACK;
extern unsigned int MAIN_GUI_ID;
extern SIE_GUI_STACK *GUI_STACK;

char *GetUniqueFileName(const char *file_name) {
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
            mfree(dest_path);
            return dest;
        } else {
            i += 1;
        }
    }
}

char *GetPath(const char *file_name) {
    char *path = malloc(strlen(PATH_STACK->dir_name) + strlen(file_name) + 1);
    sprintf(path, "%s%s", PATH_STACK->dir_name, file_name);
    return path;
}

void Redraw(const char *file_name) {
    size_t len = strlen(file_name);
    WSHDR *ws = AllocWS(len);
    str_2ws(ws, file_name, len);
    IPC_SetRowByFileName_ws(ws);
}

void MsgProc(int flag, void *data) {
    SIE_FILE *top = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    SIE_FILE *p = Sie_FS_DeleteFileElement(top, (SIE_FILE*)data);
    if (p) {
        char *src = Sie_FS_GetPathByFile(p);
        if (flag == SIE_GUI_MSG_BOX_CALLBACK_YES) {
            char *unique = GetUniqueFileName(p->file_name);
            char *dest_path = GetPath(unique);
            if (p->file_attr & FA_DIRECTORY) {
                // CopyFileRecursive
                ShowMSG(1, (int)"Is directory");
            } else {
                Sie_FS_CopyFile(src, dest_path);
            }
            Redraw(unique);
            mfree(unique);
            mfree(dest_path);
        }
        mfree(src);
        if (!p->prev && !p->next) {
            COPY_FILES = MOVE_FILES = NULL;
        }
        Sie_FS_DestroyFileElement(p);
    }
}

void Paste() {
    SIE_FILE *top = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    if (top) {
        char *dir_name = PATH_STACK->dir_name;
        SIE_FILE *p = top;
        while (1) {
            SIE_FILE *next = p->next;
            if (strcmpi(dir_name, p->dir_name) == 0) { // current dir
                char *unique = GetUniqueFileName(p->file_name);
                char *dest_path = GetPath(unique);
                if (COPY_FILES) {
                    char *src = Sie_FS_GetPathByFile(p);
                    Sie_FS_CopyFile(src, dest_path);
                    mfree(src);
                }
                mfree(dest_path);
                if (!next) {
                    Redraw(unique);
                    mfree(unique);
                    Sie_FS_DestroyFiles(COPY_FILES);
                    COPY_FILES = NULL;
                    break;
                }
                mfree(unique);
            } else {
                char *dest = GetPath(p->file_name);
                if (Sie_FS_FileExists(dest)) {
                    mfree(dest);
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
                        char *src = Sie_FS_GetPathByFile(p);
                        Sie_FS_CopyFile(src, dest);
                        mfree(src);
                        mfree(dest);
                        if (!next) {
                            Redraw(top->file_name);
                            Sie_FS_DestroyFiles(COPY_FILES);
                            COPY_FILES = NULL;
                            break;
                        }
                    } else {
                        char *src = Sie_FS_GetPathByFile(p);
                        fmove(src, dest, &err);
                        mfree(src);
                        mfree(dest);
                        if (!next) {
                            Redraw(top->file_name);
                            Sie_FS_DestroyFiles(MOVE_FILES);
                            MOVE_FILES = NULL;
                            break;
                        }
                    }
                }
            }
            p = next;
        }
    }
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
}
