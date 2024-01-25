#include <swilib.h>
#include "sort.h"
#include "../ipc.h"
#include "../helpers.h"

extern unsigned int SORT;

void Sort(void *item, unsigned int row) {
    SORT = row;
    CloseChildrenGUI();
    IPC_Reload();
}
