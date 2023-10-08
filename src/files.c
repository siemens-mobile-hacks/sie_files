#include <stdlib.h>
#include "files.h"

extern file_t CURRENT_FILE;

files_list_t *InitFilesListFromCurrentFile() {
    files_list_t *files = malloc(sizeof(files_list_t));
    zeromem(files, sizeof(files_list_t));
    files->file = malloc(sizeof(file_t));
    memcpy(files->file, &CURRENT_FILE, sizeof(file_t));
    return files;
}

void DestroyFilesList(files_list_t *files) {
    files_list_t *p = files;
    while (p != NULL) {
        files_list_t *next = p->next;
        mfree(p->file);
        mfree(p);
        p = next;
    }
}
