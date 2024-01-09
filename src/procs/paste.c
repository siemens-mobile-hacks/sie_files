#include <swilib.h>
#include <stdlib.h>
#include <sie/sie.h>
#include "../ipc.h"
#include "../helpers.h"
#include "../path_stack.h"

extern SIE_GUI_STACK *GUI_STACK;
extern path_stack_t *PATH_STACK;
extern SIE_FILE *COPY_FILES, *MOVE_FILES;

unsigned int WAIT;
unsigned int COUNT;
SIE_GUI_BOX_GUI *BOX_GUI;
char LAST_FILE_NAME[512];

static char *GetMsg(unsigned int id) {
    static char msg[64];
    sprintf(msg, "%s: %d/%d", (COPY_FILES) ? "Копирование" : "Перемещение", id, COUNT);
    return msg;
}

static void BoxProc(int flag, void *data) {
    SIE_FILE *file = (SIE_FILE*)data;
    char *src = Sie_FS_GetPathByFile(file);
    if (flag == SIE_GUI_BOX_CALLBACK_YES) {
        SIE_FILE *new_file = GetUniqueFileInCurrentDir(file);
        char *dest = Sie_FS_GetPathByFile(new_file);
        if (new_file->file_attr & SIE_FS_FA_DIRECTORY) {
            ShowMSG(1, (int) "Is directory");
        } else {
            Sie_FS_CopyFile(src, dest);
        }
        mfree(dest);
        strcpy(LAST_FILE_NAME, new_file->file_name);
        Sie_FS_DestroyFileElement(new_file);
    }
    mfree(src);
    WAIT = 2;
}

static void Update(SIE_FILE *file, unsigned int id) {
    wsprintf(BOX_GUI->msg_ws, "%t", GetMsg(id + 1));
    PendedRedrawGUI();
}

static void SUBPROC_Paste(void) {
    unsigned int id = 0;
    SIE_FILE *files = (COPY_FILES) ? COPY_FILES : MOVE_FILES;
    COUNT = Sie_FS_GetFilesCount(files);

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
            SIE_FILE *new_file = GetUniqueFileInCurrentDir(file);
            char *dest = Sie_FS_GetPathByFile(new_file);
            if (COPY_FILES) {
                char *src = Sie_FS_GetPathByFile(file);
                Sie_FS_CopyFile(src, dest);
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
                SIE_GUI_BOX_GUI *box_gui = Sie_GUI_Box("Файл существует",
                                                       "Вставить", "Заменить", &callback);
                GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(box_gui->gui), box_gui->gui_id);
                WAIT = 1;
                continue;
            } else {
                char *src = Sie_FS_GetPathByFile(file);
                if (COPY_FILES) {
                    Sie_FS_CopyFile(src, dest);
                } else {
                    unsigned int err;
                    fmove(src, dest, &err);
                }
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

    size_t len = strlen(LAST_FILE_NAME);
    if (len) {
        WSHDR *ws = AllocWS(len);
        str_2ws(ws, LAST_FILE_NAME, len);
        IPC_CloseChildrenGUI(1);
        IPC_SetRowByFileName_ws(ws);
    }
    BOX_GUI = NULL;
    Sie_FS_DestroyFiles(files);
    COPY_FILES = MOVE_FILES = NULL;
}

unsigned int IsPasteAllow() {
    return ((COPY_FILES || (MOVE_FILES && strcmpi(MOVE_FILES->dir_name, PATH_STACK->dir_name))) &&
            strlen(PATH_STACK->dir_name));
}

void Paste(void) {
    if (IsPasteAllow()) {
        LAST_FILE_NAME[0] = '\0';
        BOX_GUI = Sie_GUI_MsgBox(GetMsg(0));
        SUBPROC(SUBPROC_Paste);
        GUI_STACK = Sie_GUI_Stack_Add(GUI_STACK, &(BOX_GUI->gui), BOX_GUI->gui_id);
    }
}
