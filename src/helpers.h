#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <sie/sie.h>

SIE_FILE *GetUniqueFileInCurrentDir(SIE_FILE *file);
unsigned int IsSelectedCurrentFile();
unsigned int IsAllowPaste();

#endif
