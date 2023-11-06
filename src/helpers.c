#include <swilib.h>
#include <stdlib.h>
#include "helpers.h"
#include "path_stack.h"

extern path_stack_t *PATH_STACK;

SIE_FILE *GetUniqueFileInCurrentDir(SIE_FILE *file) {
    SIE_FILE *f = Sie_FS_CopyFileElement(file);
    mfree(f->dir_name);
    f->dir_name = malloc(strlen(PATH_STACK->dir_name + 1));
    strcpy(f->dir_name, PATH_STACK->dir_name);

    SIE_FILE *unique = Sie_FS_GetUniqueFile(f);
    Sie_FS_DestroyFileElement(f);
    return unique;
}
