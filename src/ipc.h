#ifndef __IPC_H__
#define __IPC_H__

#define IPC_NAME "SieFiles"

#define IPC_RELOAD                  0x00
#define IPC_SET_ROW_BY_FILE_NAME_WS 0x01

void IPC_Reload();
void IPC_SetRowByFileName_ws(WSHDR *ws);

#endif
