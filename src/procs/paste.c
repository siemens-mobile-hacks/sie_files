#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"
#include "../path_stack.h"

extern SIE_FILE *COPY_FILES, *MOVE_FILES;
extern path_stack_t *PATH_STACK;

void Redraw(const char *file_name) {
    size_t len = strlen(file_name);
    WSHDR *ws = AllocWS(len);
    str_2ws(ws, file_name, len);
    IPC_SetRowByFileName_ws(ws);
}

static void MsgProc(int flag, void *data) {
    SIE_FILE *top = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    SIE_FILE *p = Sie_FS_DeleteFileElement(top, (SIE_FILE*)data);
    if (p) {
        char *src = Sie_FS_GetPathByFile(p);
        if (flag == SIE_GUI_BOX_CALLBACK_NO) {
            SIE_FILE *file = GetUniqueFileInCurrentDir(p);
            char *path = Sie_FS_GetPathByFile(file);
            if (p->file_attr & SIE_FS_FA_DIRECTORY) {
                // CopyFileRecursive
                ShowMSG(1, (int)"Is directory");
            } else {
                Sie_FS_CopyFile(src, path);
            }
            Redraw(file->file_name);
            mfree(path);
            Sie_FS_DestroyFileElement(file);
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
                SIE_FILE *file = GetUniqueFileInCurrentDir(p);
                char *dest = Sie_FS_GetPathByFile(file);
                if (COPY_FILES) {
                    char *src = Sie_FS_GetPathByFile(p);
                    Sie_FS_CopyFile(src, dest);
                    mfree(src);
                }
                mfree(dest);
                if (!next) {
                    Redraw(file->file_name);
                    Sie_FS_DestroyFileElement(file);
                    Sie_FS_DestroyFiles(COPY_FILES);
                    COPY_FILES = NULL;
                    break;
                }
                Sie_FS_DestroyFileElement(file);
            } else {
                char *dest = malloc(strlen(PATH_STACK->dir_name) + strlen(p->file_name) + 1);
                sprintf(dest, "%s%s", PATH_STACK->dir_name, p->file_name);
                if (Sie_FS_FileExists(dest)) {
                    mfree(dest);
                    SIE_GUI_BOX_CALLBACK callback;
                    callback.proc = MsgProc;
                    callback.data = p;
                    Sie_GUI_Box("Файл существует", "Заменить", "Вставить", &callback);
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
    IPC_CloseChildrenGUI(0);
}
