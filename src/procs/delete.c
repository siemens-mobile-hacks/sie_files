#include <swilib.h>
#include <stdlib.h>
#include <nu_swilib.h>
#include <sie/sie.h>
#include "procs.h"
#include "../ipc.h"
#include "../msg.h"
#include "../helpers.h"

extern SIE_GUI_SURFACE *SURFACE;
extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *SELECTED_FILES;
extern SIE_GUI_STACK *GUI_STACK;

static int COUNT;
static SIE_GUI_BOX *BOX_GUI;

static char *GetMsg(unsigned int id) {
    static char msg[64];
    sprintf(msg, "Deleting files: %d/%d", id, COUNT);
    return msg;
}

static void DeleteFiles(SIE_FILE *files) {
    unsigned int err = 0;
    unsigned int i = 0;
    SIE_FILE *file = (SIE_FILE*)files;
    while (file) {
        char *path = Sie_FS_GetPathByFile(file);
        if (file->file_attr & SIE_FS_FA_DIRECTORY) {
            if (!Sie_FS_DeleteDirRecursive(path, &err)) {
                MsgBoxError_FileAction(file, "delete");
            }
        } else {
            if (!Sie_FS_DeleteFile(path, &err)) {
                MsgBoxError_FileAction(file, "delete");
            }
        }
        mfree(path);
        Sie_GUI_BoxUpdate(BOX_GUI, GetMsg(++i));
        file = file->next;
    }
}

static void SubProc_Delete(SIE_FILE *files) {   
    DeleteFiles(files);
    Sie_FS_DestroyFiles(files);
    Sie_GUI_BoxClose(BOX_GUI);
    IPC_Reload();
    BOX_GUI = NULL;
}

static void BoxProc(int flag, void *data) {
    if (flag == SIE_GUI_BOX_CALLBACK_YES) {
        BOX_GUI = Sie_GUI_WaitBox(GetMsg(0), SURFACE->scrot);
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
        Sie_GUI_Surface_TakeScrot(SURFACE);
        Sie_GUI_MsgBoxYesNo("Delete?", &callback, SURFACE->scrot);
    }
}
