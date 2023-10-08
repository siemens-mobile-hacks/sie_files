#include <swilib.h>
#include "ipc.h"

void ipc_redraw() {
    static IPC_REQ ipc;
    zeromem(&ipc, sizeof(IPC_REQ));
    ipc.name_from = IPC_NAME;
    ipc.name_to = IPC_NAME;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, IPC_REDRAW, &ipc);
}
