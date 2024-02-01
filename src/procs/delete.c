#include <sie/sie.h>
#include "procs.h"
#include "../ipc.h"
#include "../helpers.h"

extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *SELECTED_FILES;
extern SIE_GUI_STACK *GUI_STACK;

int COUNT;
static SIE_GUI_BOX_GUI *BOX_GUI;

static char *GetMsg(SIE_FILE *files, unsigned int id) {
    static char msg[64];
    sprintf(msg, "Deleting files: %d/%d", id, COUNT);
    return msg;
}

static void Update(SIE_FILE *files, SIE_FILE *file, unsigned int id) {
    wsprintf(BOX_GUI->msg_ws, "%t", GetMsg(files, id + 1));
    PendedRedrawGUI();
}

static void DeleteFiles(SIE_FILE *files) {
    unsigned int err;
    unsigned int i = 0;
    SIE_FILE *file = (SIE_FILE*)files;
    while (file) {
        char *path = Sie_FS_GetPathByFile(file);
        if (file->file_attr & SIE_FS_FA_DIRECTORY) {
            Sie_FS_DeleteDirRecursive(path, &err);
        } else {
            Sie_FS_DeleteFile(path, &err);
        }
        mfree(path);
        Update(files, file, i++);
        file = file->next;
    }
}

static void SubProc_Delete(SIE_FILE *files) {   
    DeleteFiles(files);
    Sie_FS_DestroyFiles(files);
    Sie_GUI_CloseGUI_GBS(BOX_GUI->surface->gui_id);
    IPC_Reload();
    BOX_GUI = NULL;
}

static void BoxProc(int flag, void *data) {
    if (flag == SIE_GUI_BOX_CALLBACK_YES) {
        BOX_GUI = Sie_GUI_MsgBox(GetMsg(SELECTED_FILES, 0));
        Sie_GUI_Stack_Add(GUI_STACK, &(BOX_GUI->gui), BOX_GUI->surface->gui_id);
        Sie_SubProc_Run(SubProc_Delete, data);
        SELECTED_FILES = NULL;
    }
}

void Delete() {
    SIE_FILE *files = NULL;
    if (SELECTED_FILES) {
        files = SELECTED_FILES;
    } else if (CURRENT_FILE && !(CURRENT_FILE->file_attr & SIE_FS_FA_VOLUME)) {
        files = Sie_FS_CopyFileElement(CURRENT_FILE);
    }
    if (files) {
        SIE_GUI_BOX_CALLBACK callback;
        zeromem(&callback, sizeof(SIE_GUI_BOX_CALLBACK));
        callback.proc = BoxProc;
        callback.data = files;
        COUNT = Sie_FS_GetFilesCount(files);
        CloseChildrenGUI();
        Sie_GUI_MsgBoxYesNo("Delete?", &callback);
    }
}
