#include <sie/sie.h>
#include "procs.h"
#include "../ipc.h"

extern SIE_FILE *CURRENT_FILE;
extern SIE_GUI_STACK *GUI_STACK;

SIE_GUI_BOX_GUI *BOX_GUI;

static char *GetMsg(unsigned int id) {
    static char msg[64];
    static int count = 0;
    if (!count) {
        count = Sie_FS_GetFilesCount(CURRENT_FILE);
    }
    sprintf(msg, "Удаление файлов: %d/%d", id, count);
    return msg;
}

void DeleteFiles(const SIE_FILE *files, void (*proc)(SIE_FILE *file, unsigned int id)) {
    unsigned int err;
    unsigned int i = 0;
    SIE_FILE *file = (SIE_FILE*)files;
    while (file) {
        char *path = Sie_FS_GetPathByFile(file);
        if (file->file_attr & SIE_FS_FA_DIRECTORY) {
            Sie_FS_RemoveDirRecursive(path);
        } else {
            _unlink(path, &err);
        }
        mfree(path);
        if (proc) {
            proc(file, i++);
        }
        file = file->next;
    }
}

// CALLBACK HELL BELLOW

static void DeleteFileProc(SIE_FILE *file, unsigned int id) {
    wsprintf(BOX_GUI->msg_ws, "%t", GetMsg(id + 1));
    PendedRedrawGUI();
}

static void DeleteProc() {
    SIE_FILE *files = Sie_FS_CopyFileElement(CURRENT_FILE);
    DeleteFiles(files, DeleteFileProc);
    Sie_FS_DestroyFiles(files);
    IPC_CloseChildrenGUI(1);
    BOX_GUI = NULL;
}

static void MsgProc(int flag, void *data) {
    if (flag == SIE_GUI_BOX_CALLBACK_YES) {
        BOX_GUI = Sie_GUI_MsgBox(GetMsg(0));
        GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(BOX_GUI->gui), BOX_GUI->gui_id);
        SUBPROC(DeleteProc);
    }
}

void Delete() {
    SIE_GUI_BOX_CALLBACK callback;
    zeromem(&callback, sizeof(SIE_GUI_BOX_CALLBACK));
    callback.proc = MsgProc;
    Sie_GUI_MsgBoxYesNo("Удалить?", &callback);
}
