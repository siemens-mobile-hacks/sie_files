#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include "path_stack.h"

path_stack_t *InitPathStack(void) {
    path_stack_t *path_stack = malloc(sizeof(path_stack_t));
    zeromem(path_stack, sizeof(path_stack_t));
    path_stack->dir_name = malloc(1);
    path_stack->dir_name[0] = '\0';
    return path_stack;
}

void DestroyPathStack(path_stack_t *path_stack) {
    while (path_stack) {
        path_stack_t *prev = path_stack->prev;
        mfree(path_stack->dir_name);
        mfree(path_stack);
        path_stack = prev;
    }
}

path_stack_t *PathStack_Pop(path_stack_t *path_stack) {
    path_stack_t *prev = path_stack->prev;
    mfree(path_stack->dir_name);
    mfree(path_stack);
    prev->next = NULL;
    return prev;
}

path_stack_t *PathStack_Add(path_stack_t *path_stack, const char *path) {
    size_t len_path = strlen(path);
    path_stack_t *top = malloc(sizeof(path_stack_t));
    zeromem(top, sizeof(path_stack_t));
    top->prev = path_stack;
    top->dir_name = malloc(len_path + 1);
    strcpy(top->dir_name, path);
    path_stack->next = top;
    return top;
}
