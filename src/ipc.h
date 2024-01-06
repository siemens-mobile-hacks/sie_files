#ifndef __IPC_H__
#define __IPC_H__

#define IPC_NAME "SieFiles"

#define IPC_REDRAW                  0x00
#define IPC_CLOSE_CHILDREN_GUI      0x01
#define IPC_SET_ROW_BY_FILE_NAME_WS 0x02

void IPC_Redraw();
void IPC_CloseChildrenGUI(unsigned int redraw);
void IPC_SetRowByFileName_ws(WSHDR *ws);

#endif
