#ifndef __PATH_STACK_H__
#define __PATH_STACK_H__

typedef struct {
    void *next;
    void *prev;
    char *dir_name;
    unsigned int row;
} _PATH_STACK;

typedef _PATH_STACK path_stack_t;

path_stack_t *InitPathStack(void);
void DestroyPathStack(path_stack_t *path_stack);

path_stack_t *PathStack_Pop(path_stack_t *path_stack);
path_stack_t *PathStack_Add(path_stack_t *path_stack, const char *path);

#endif
