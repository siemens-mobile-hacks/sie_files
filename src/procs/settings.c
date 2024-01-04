#include <sie/sie.h>
#include "../ipc.h"

extern unsigned int MAIN_GUI_ID;
extern SIE_GUI_STACK *GUI_STACK;
extern unsigned int SHOW_HIDDEN_FILES;

void ToggleHiddenFiles(SIE_MENU_LIST_ITEM *item, unsigned int row) {
    item->flag = !item->flag;
    SHOW_HIDDEN_FILES = item->flag;
    GUI_STACK = Sie_GUI_Stack_CloseChildren(GUI_STACK, MAIN_GUI_ID);
    IPC_Redraw();
}
