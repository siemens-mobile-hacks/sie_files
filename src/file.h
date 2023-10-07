#ifndef __FILE_H__
#define __FILE_H__

#include <sie/sie.h>

typedef struct {
    const char *dir;
    const SIE_FILE *sie_file;
} _FILE;

typedef _FILE file_t;

#endif
