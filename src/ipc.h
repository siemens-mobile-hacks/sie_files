#ifndef __IPC_H__
#define __IPC_H__

#define IPC_NAME "SieFiles"

#define IPC_RELOAD               0x00
#define IPC_SET_ROW_BY_FILE_NAME 0x01

typedef struct {
    void *data;
    unsigned int gui_id;
} IPC_DATA;

void IPC_Reload();
void IPC_SetRowByFileName(const char *file_name);

#endif
