#include <sie/sie.h>
#include "procs.h"

extern SIE_FILE *CURRENT_FILE;

static void MsgProc(int flag, void *data) {
    if (flag == SIE_GUI_MSG_BOX_CALLBACK_YES) {
        SIE_FILE *files = Sie_FS_CopyFileElement(CURRENT_FILE);
        DeleteFiles(files);
        Sie_FS_DestroyFiles(files);
    }
}

void Delete(void) {
    SIE_GUI_MSG_BOX_CALLBACK callback;
    zeromem(&callback, sizeof(SIE_GUI_MSG_BOX_CALLBACK));
    callback.proc = MsgProc;
    Sie_GUI_MsgBoxYesNo("Удалить?", &callback);
}
