#include <swilib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"

extern SIE_FILE *CURRENT_FILE;
extern SIE_FILE *SELECTED_FILES;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;

static void CopyOrMoveFiles(unsigned int type) {
    SIE_FILE *files = NULL;
    if (SELECTED_FILES) {
        files = Sie_FS_CloneFiles(SELECTED_FILES);
        Sie_FS_DestroyFiles(SELECTED_FILES);
        SELECTED_FILES = NULL;
    } else if (CURRENT_FILE && !(CURRENT_FILE->file_attr & SIE_FS_FA_VOLUME)) {
        files = Sie_FS_CopyFileElement(CURRENT_FILE);
    }
    if (files) {
        if (type == 0) {
            COPY_FILES = files;
        } else {
            MOVE_FILES = files;
        }
        CloseChildrenGUI();
        IPC_Reload();
    } else {
        CloseChildrenGUI();
    }
}

void CopyFiles() {
    CopyOrMoveFiles(0);
}

void MoveFiles() {
    CopyOrMoveFiles(1);
}
