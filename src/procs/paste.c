#include <swilib.h>
#include <stdlib.h>
#include <nu_swilib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"
#include "../path_stack.h"

typedef struct {
    int flag;
    void *data;
} SUBPROC_DATA;

extern path_stack_t *PATH_STACK;
extern SIE_GUI_STACK *GUI_STACK;
extern unsigned int MAIN_GUI_ID;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;

volatile unsigned int WAIT;
static unsigned int COUNT;
static SIE_GUI_BOX_GUI *BOX_GUI;
char LAST_FILE_NAME[512];

static char *GetMsg(unsigned int id) {
    static char msg[64];
    sprintf(msg, "%s: %d/%d", (COPY_FILES) ? "Copying" : "Moving", id, COUNT);
    return msg;
}

static void CopyOrMoveFile(const char *src, const char *dest) {
    unsigned int err = 0;
    if (COPY_FILES) {
        if (Sie_FS_IsDir(src, &err)) {
            Sie_FS_CopyDir(src, dest, &err);
        } else {
            Sie_FS_CopyFile(src, dest, &err);
        }
    } else {
        Sie_FS_MoveFile(src, dest, &err);
    }
}

static void SubProc_Box(SUBPROC_DATA *data) {
    unsigned int err;
    unsigned int flag = data->flag;
    SIE_FILE *file = (SIE_FILE*)data->data;
    SIE_FILE *new_file = NULL;
    char *src = Sie_FS_GetPathByFile(file);
    if (flag == SIE_GUI_BOX_CALLBACK_YES) {
        new_file = GetUniqueFileInCurrentDir(file);
        char *dest = Sie_FS_GetPathByFile(new_file);
        CopyOrMoveFile(src, dest);
        mfree(dest);
    } else {
        new_file = Sie_FS_CopyFileElement(file);
        mfree(new_file->dir_name);
        new_file->dir_name = malloc(strlen(PATH_STACK->dir_name) + 1);
        strcpy(new_file->dir_name, PATH_STACK->dir_name);

        char *dest = Sie_FS_GetPathByFile(new_file);
        if (new_file->file_attr & SIE_FS_FA_DIRECTORY) {
            Sie_FS_DeleteDirRecursive(dest, &err);
            CopyOrMoveFile(src, dest);
        } else {
            Sie_FS_DeleteFile(dest, &err);
            CopyOrMoveFile(src, dest);
        }
        mfree(dest);
    }
    mfree(src);
    strcpy(LAST_FILE_NAME, new_file->file_name);
    Sie_FS_DestroyFileElement(new_file);
    WAIT = 2;
}

static void BoxProc(int flag, void *data) {
    static SUBPROC_DATA subproc_data;
    subproc_data.flag = flag;
    subproc_data.data = data;
    Sie_SubProc_Run(SubProc_Box, &subproc_data);
}

static void Update(SIE_FILE *file, unsigned int id) {
    wsprintf(BOX_GUI->msg_ws, "%t", GetMsg(id + 1));
    PendedRedrawGUI();
}

static void SubProc_Paste() {
    unsigned int id = 0;
    SIE_FILE *files = (COPY_FILES) ? COPY_FILES : MOVE_FILES;

    SIE_FILE *file = files;
    while (1) {
        if (WAIT == 1) {
            NU_Sleep(50);
            continue;
        } else if (WAIT == 2) {
            WAIT = 0;
            goto END;
        }

        const char *dir_name = PATH_STACK->dir_name;
        if (strcmp(file->dir_name, dir_name) == 0) { // current dir
            unsigned int err = 0;
            SIE_FILE *new_file = GetUniqueFileInCurrentDir(file);
            char *dest = Sie_FS_GetPathByFile(new_file);
            if (COPY_FILES) {
                char *src = Sie_FS_GetPathByFile(file);
                if (Sie_FS_IsDir(src, &err)) {
                    Sie_FS_CopyDir(src, dest, &err);
                } else {
                    Sie_FS_CopyFile(src, dest, &err);
                }
                mfree(src);
            }
            mfree(dest);
            strcpy(LAST_FILE_NAME, new_file->file_name);
            Sie_FS_DestroyFileElement(new_file);
        } else {
            char *dest = malloc(strlen(dir_name) + strlen(file->file_name) + 1);
            sprintf(dest, "%s%s", dir_name, file->file_name);
            if (Sie_FS_FileExists(dest)) {
                SIE_GUI_BOX_CALLBACK callback;
                callback.proc = BoxProc;
                callback.data = file;
                Sie_GUI_Box("File exists","Paste", "Replace", &callback);
                WAIT = 1;
                continue;
            } else {
                char *src = Sie_FS_GetPathByFile(file);
                CopyOrMoveFile(src, dest);
                mfree(src);
                strcpy(LAST_FILE_NAME, file->file_name);
            }
            mfree(dest);
        }
        END:
        Update(file, id++);
        if (file->next) {
            file = file->next;
        } else break;
    }

    Sie_GUI_CloseGUI_GBS(BOX_GUI->surface->gui_id);

    size_t len = strlen(LAST_FILE_NAME);
    if (len) {
        WSHDR *ws = AllocWS(len);
        str_2ws(ws, LAST_FILE_NAME, len);
        IPC_Reload();
        IPC_SetRowByFileName_ws(ws);
    }
    Sie_FS_DestroyFiles(files);
    COPY_FILES = MOVE_FILES = NULL;

    BOX_GUI = NULL;
}

unsigned int IsPasteAllow() {
    return ((COPY_FILES || (MOVE_FILES && strcmpi(MOVE_FILES->dir_name, PATH_STACK->dir_name))) &&
            strlen(PATH_STACK->dir_name));
}

static void Proc() {
    LAST_FILE_NAME[0] = '\0';
    COUNT = Sie_FS_GetFilesCount((COPY_FILES) ? COPY_FILES : MOVE_FILES);
    BOX_GUI = Sie_GUI_MsgBox(GetMsg(0));
    Sie_SubProc_Run(SubProc_Paste, NULL);
}

void Paste(void) {
    if (IsPasteAllow()) {
        static GBSTMR tmr;
        static SIE_GUI_FOCUS_DATA data;
        CloseChildrenGUI();
        data.gui_id = MAIN_GUI_ID;
        data.proc = Proc;
        data.data = NULL;
        Sie_GUI_FocusGUI(&tmr, &data);
    }
}
