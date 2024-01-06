#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"
#include "../path_stack.h"

extern SIE_GUI_STACK *GUI_STACK;
extern path_stack_t *PATH_STACK;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;

unsigned int WAIT = 0;
SIE_GUI_BOX_GUI *BOX_GUI;

static char *GetMsg(unsigned int id) {
    static char msg[64];
    static int count = 0;
    if (!count) {
        count = (COPY_FILES) ? Sie_FS_GetFilesCount(COPY_FILES) : Sie_FS_GetFilesCount(MOVE_FILES);
    }
    sprintf(msg, "%s: %d/%d", (COPY_FILES) ? "Копирование" : "Перемещение", id, count);
    return msg;
}

static void BoxProc(int flag, void *data) {
    SIE_FILE *file = (SIE_FILE*)data;
    char *src = Sie_FS_GetPathByFile(file);
    if (flag == SIE_GUI_BOX_CALLBACK_NO) {
        SIE_FILE *new_file = GetUniqueFileInCurrentDir(file);
        char *dest = Sie_FS_GetPathByFile(new_file);
        if (new_file->file_attr & SIE_FS_FA_DIRECTORY) {
            ShowMSG(1, (int) "Is directory");
        } else {
            Sie_FS_CopyFile(src, dest);
        }
        mfree(dest);
        Sie_FS_DestroyFileElement(new_file);
    }
    mfree(src);
    WAIT = 2;
}

static void Update(SIE_FILE *file, unsigned int id) {
    wsprintf(BOX_GUI->msg_ws, "%t", GetMsg(id + 1));
    PendedRedrawGUI();
}

static void SUBPROC_Paste(void) {
    unsigned int id = 0;
    SIE_FILE *file = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    while (1) {
        if (WAIT == 1) {
            NU_Sleep(50);
            continue;
        } else if (WAIT == 2) {
            WAIT = 0;
            goto END;
        }

        const char *dir_name = PATH_STACK->dir_name;
        if (strcmpi(file->dir_name, dir_name) == 0) { // current dir
            SIE_FILE *new_file = GetUniqueFileInCurrentDir(file);
            char *dest = Sie_FS_GetPathByFile(new_file);
            if (COPY_FILES) {
                char *src = Sie_FS_GetPathByFile(new_file);
                Sie_FS_CopyFile(src, dest);
                mfree(src);
            }
            mfree(dest);
        } else {
            char *dest = malloc(strlen(dir_name) + strlen(file->file_name) + 1);
            sprintf(dest, "%s%s", dir_name, file->file_name);
            if (Sie_FS_FileExists(dest)) {
                SIE_GUI_BOX_CALLBACK callback;
                callback.proc = BoxProc;
                callback.data = file;
                SIE_GUI_BOX_GUI *box_gui = Sie_GUI_Box("Файл существует",
                                                       "Заменить", "Вставить", &callback);
                GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(box_gui->gui), box_gui->gui_id);
                WAIT = 1;
                continue;
            } else {
                char *src = Sie_FS_GetPathByFile(file);
                if (COPY_FILES) {
                    Sie_FS_CopyFile(src, dest);
                    Sie_FS_DestroyFiles(COPY_FILES);
                    COPY_FILES = NULL;
                } else {
                    unsigned int err;
                    fmove(src, dest, &err);
                }
                mfree(src);
            }
            mfree(dest);
        }

        END:
        Update(file, id++);
        if (file->next) {
            file = file->next;
        } else break;
    }

    size_t len = strlen(file->file_name);
    WSHDR *ws = AllocWS(len);
    str_2ws(ws, file->file_name, len);
    IPC_CloseChildrenGUI(1);
    IPC_SetRowByFileName_ws(ws);
    COPY_FILES = MOVE_FILES = NULL;
}

void Paste(void) {
    BOX_GUI = Sie_GUI_MsgBox(GetMsg(0));
    SUBPROC(SUBPROC_Paste);
    GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(BOX_GUI->gui), BOX_GUI->gui_id);
}
