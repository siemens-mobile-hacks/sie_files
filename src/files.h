#ifndef __FILES_H__
#define __FILES_H__

#include <sie/sie.h>

typedef struct {
    const char *dir;
    const SIE_FILE *sie_file;
} _FILE;

typedef _FILE file_t;

typedef struct {
    file_t *file;
    void *next;
} _FILES;

typedef _FILES files_list_t;

files_list_t *InitFilesListFromCurrentFile();
void DestroyFilesList(files_list_t *files);

#endif
