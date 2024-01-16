#include <swilib.h>
#include "ipc.h"

static void Send(IPC_REQ *ipc, unsigned int msg) {
    ipc->name_from = IPC_NAME;
    ipc->name_to = IPC_NAME;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, msg, ipc);
}

void IPC_Reload() {
    static IPC_REQ ipc;
    ipc.data = NULL;
    Send(&ipc, IPC_RELOAD);
}

void IPC_SetRowByFileName_ws(WSHDR *ws) {
    static IPC_REQ ipc;
    ipc.data = (void*)ws;
    Send(&ipc, IPC_SET_ROW_BY_FILE_NAME_WS);
}
