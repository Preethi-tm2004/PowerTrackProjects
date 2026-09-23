/*
    DESCRIPTION : This file contains the implementation of the dynamically resizable stacks used for Undo and Redo operations.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "undo_redo.h"


/* Initial size of the stack. The stack will automatically grow when this capacity is reached */
#define INITIAL_CAPACITY 5

/* Initialize an empty stack. top = -1 means that the stack currently contains no operations */
void initStack(Stack *stack)
{
    stack->top = -1;
    stack->capacity = INITIAL_CAPACITY;

    stack->operations = malloc(stack->capacity * sizeof(Operation));

    if(stack->operations == NULL)
    {
        stack->capacity = 0;
    }
}

/* Check whether the stack is empty */
int isStackEmpty(Stack *stack)
{
    return stack->top == -1;
}

/* Add an operation to the top of the stack. If the stack is full, its capacity is doubled before adding the new operation */
int push(Stack *stack, Operation operation)
{
    Operation *temp;

    /* Allocate the initial stack if initialization failed. */
    if (stack->capacity == 0)
    {
        stack->capacity = INITIAL_CAPACITY;

        stack->operations = malloc(stack->capacity * sizeof(Operation));

        if (stack->operations == NULL)
        {
            stack->capacity = 0;
            return 0;
        }
    }


    /* Resize the stack when it becomes full */
    if(stack->top == stack->capacity - 1)
    {
        int newCapacity = stack->capacity * 2;

        temp = realloc(stack->operations, newCapacity * sizeof(Operation));

        if(temp == NULL)
        {
            return 0;
        }
        stack->operations = temp;
        stack->capacity = newCapacity;
    }

    stack->top++;
    stack->operations[stack->top] = operation;

    return 1;
}

/* Remove the top operation from the stack. The removed operation is copied into the variable provided by the caller */
int pop(Stack *stack, Operation *operation)
{
    if(isStackEmpty(stack))
    {
        return 0;
    }

    *operation = stack->operations[stack->top];

    stack->top--;

    return -1;
}

/* Free all memory allocated for the stack */
void freeStack(Stack *stack)
{
    for(int i = 0; i <= stack->top; i++)
    {
        free(stack->operations[i].text);
    }

    free(stack->operations);

    stack->operations = NULL;
    stack->top = -1;
    stack->capacity = 0;
}

/* Remove all operations currently stored in the stack. The stack itself is kept allocated so it can be reused for future operations */
void clearStack(Stack *stack)
{
    int i;
    for(i = 0; i <= stack->top; i++)
    {
        free(stack->operations[i].text);
    }

    stack->top = -1;
}