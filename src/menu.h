#ifndef __MENU_H__
#define __MENU_H__

#include <sie/sie.h>

SIE_MENU_LIST *InitMenu(unsigned int gui_id, const char **names, void (**procs)(void), unsigned int count);
void DestroyMenu(SIE_MENU_LIST *menu);
#define M_InitMenu() InitMenu(data->gui_id, (const char **)names, procs, count)

void AddMenuItem(char ***names, void (***procs)(), unsigned int *count, const char *name, void (*proc)());
void DestroyMenuItems(char **names, void (**procs)(), unsigned int count);
#define M_AddMenuItem(name, proc) AddMenuItem(&names, &procs, &count, name, proc)
#define M_DestroyMenuItems() DestroyMenuItems(names, procs, count)

#endif
