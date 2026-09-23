#ifndef UNDO_REDO_H
#define UNDO_REDO_H

/* Type of editing operation performed by the user */
typedef enum
{
    INSERT,
    NEW_LINE,
    DELETE
}OperationType;

/*
    Stores information about one editing operation.

    type     -> INSERT or DELETE
    text     -> text affected by the operation
    line     -> line number where the operation occurred
    position -> cursor position where the operation occurred
*/
typedef struct
{
    OperationType type;

    char *text;

    int line;
    int position;

}Operation;

/*
    Dynamically resizable stack used for Undo and Redo.

    operations -> dynamically allocated array of operations
    top        -> index of the top operation
    capacity   -> current size of the allocated array
*/
typedef struct 
{
    Operation *operations;

    int top;
    int capacity;

}Stack;

/* Stack initialization and memory cleanup. */
void initStack(Stack *stack);
void freeStack(Stack *stack);

/* Stack operation */
int push(Stack *stack, Operation operation);
int pop(Stack *Stack, Operation *operation);
int isStackEmpty(Stack *editor);
void clearStack(Stack *stack);

#endif