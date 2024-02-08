#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"

extern SIE_GUI_SURFACE *SURFACE;
extern unsigned int SHOW_HIDDEN_FILES;

void Proc(void *data) {
    unsigned int flag = (unsigned int)data;
    SHOW_HIDDEN_FILES = flag;
    IPC_Reload();
}

void ToggleHiddenFiles(SIE_MENU_LIST_ITEM *item, unsigned int row) {
    static GBSTMR tmr;
    static SIE_GUI_FOCUS_DATA data;
    data.gui_id = SURFACE->gui_id;
    data.proc = Proc;
    data.data = (void*)(!item->flag);
    CloseChildrenGUI();
    Sie_GUI_FocusGUI(&tmr, &data);
}
