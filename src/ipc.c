#include <swilib.h>
#include <sie/sie.h>
#include "ipc.h"

extern SIE_GUI_SURFACE *SURFACE;

static void Send(IPC_REQ *ipc, unsigned int msg) {
    ipc->name_from = IPC_NAME;
    ipc->name_to = IPC_NAME;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, msg, ipc);
}

void IPC_Reload() {
    static IPC_REQ ipc;
    static IPC_DATA data;
    data.data = NULL;
    data.gui_id = SURFACE->gui_id;
    ipc.data = &data;
    Send(&ipc, IPC_RELOAD);
}

void IPC_SetRowByFileName(const char *file_name) {
    static IPC_REQ ipc;
    static IPC_DATA data;
    data.data = (void*)file_name;
    data.gui_id = SURFACE->gui_id;
    ipc.data = &data;
    Send(&ipc, IPC_SET_ROW_BY_FILE_NAME);
}
