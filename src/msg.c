#include <stdlib.h>
#include "msg.h"

extern SIE_GUI_SURFACE *SURFACE;

void MsgBoxError_FileAction(SIE_FILE *file, const char *action) {
    char *msg = malloc(16 + strlen(action) + 16 + strlen(file->file_name) + 1);
    sprintf(msg, "Couldn't %s ", action);
    if (file->file_attr & SIE_FS_FA_DIRECTORY) {
        strcat(msg, "directory: ");
    } else {
        strcat(msg, "file: ");
    }
    strcat(msg, file->file_name);
    Sie_GUI_MsgBoxError(msg, SURFACE->scrot);
    mfree(msg);
}
