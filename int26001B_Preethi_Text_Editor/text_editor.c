/*
    PROJECT NAME : TEXT EDITOR

    DESCRIPTION : This file contains the implementation of the core text editor operations.
*/

#include "text_editor.h"

/*
    Initialize the TextEditor structure.

    Initially the editor contains no text, so head, tail and cursor are set to NULL.

    cursorLine is initialized to 1 because we consider the first line as line number 1.

    cursorPos is initialized to 0, meaning the cursor starts at the beginning of the line.
*/
void initEditor(TextEditor *editor)
{
    editor->head = NULL;
    editor->tail = NULL;
    editor->cursor = NULL;

    editor->cursorLine = 1;
    editor->cursorPos = 0;

    /* Initialize the Undo and Redo stacks */
    initStack(&editor->undoStack);
    initStack(&editor->redoStack);
}

/* Free all nodes in the doubly linked list and release the memory used by the Undo and Redo stacks */
void freeEditor(TextEditor *editor)
{
    Node *current = editor->head;
    Node *next;

    while(current != NULL)
    {
        next = current->next;

        free(current->data);
        free(current);

        current = next;
    }

    /* Release memory used by both stacks */
    freeStack(&editor->undoStack);
    freeStack(&editor->redoStack);

    /* Reset editor pointers after freeing the memory */
    editor->head = NULL;
    editor->tail = NULL;
    editor->cursor = NULL;
}

/*
    Insert text at the current cursor position. If the editor is empty, a new line is created. 
    Otherwise, the new text is inserted into the current line at cursorPos.
*/
Status insertText(TextEditor *editor, const char *text)
{
    Node *newNode;
    Operation operation;

    int textLength;
    int currentLength;
    int newLength;

    char *newData;

    /* Do not insert an empty strcing */
    if(text == NULL || text[0] == '\0')
    {
        return FAILURE;
    }

    textLength = strlen(text);

    /* Prepare the Undo operation first. This ensures that if memory allocation fails, the editor is not modified */
    operation.type = INSERT;

    operation.text = malloc(textLength + 1);

    if (operation.text == NULL)
    {
        return FAILURE;
    }

    strcpy(operation.text, text);

    operation.line = editor->cursorLine;

     /* For an empty editor the first insertion starts at position 0 */
    if (editor->head == NULL)
    {
        operation.position = 0;
    }
    else
    {
        operation.position = editor->cursorPos;
    }

    /*  CASE 1: The editor is empty.
        Create the first node and make it the head, tail and current cursor line.
    */
    if(editor->head == NULL)
    {
        newNode = malloc(sizeof(Node));

        if(newNode == NULL)
        {
            free(operation.text);
            return FAILURE;
        }

        newNode->data = malloc(textLength + 1);

        if(newNode->data == NULL)
        {
            free(newNode);
            free(operation.text);
            return FAILURE;
        }

        strcpy(newNode->data, text);

        newNode->prev = NULL;
        newNode->next = NULL;

        editor->head = newNode;
        editor->tail = newNode;
        editor->cursor = newNode;

        editor->cursorLine = 1;

        /* After insertion, place the cursor at the end of newly inserted text */
        editor->cursorPos = textLength;
    }
    /* CASE 2: The editor already contains text. Insert the new text inside the current line */
    else
    {
        currentLength = strlen(editor->cursor->data);

        newLength = currentLength + textLength;

        newData = malloc(newLength + 1);

        if(newData == NULL)
        {
            free(operation.text);
            return FAILURE;
        }

        /* Copy the part of the old string before the cursor position */
        memcpy(newData, editor->cursor->data, editor->cursorPos);

        /* Copy the new text at the cursor position */
        memcpy(newData + editor->cursorPos, text, textLength);

        /* Copy the newly inserted text immediately after the part before the cursor */
        memcpy(newData + editor->cursorPos + textLength, editor->cursor->data + editor->cursorPos, currentLength - editor->cursorPos);

        newData[newLength] = '\0'; //Add the terminating '\0'

        /* Replace the old string with the new string */
        free(editor->cursor->data);

        editor->cursor->data = newData;

        /* Move the cursor to the end of the newly inserted text */
        editor->cursorPos = editor->cursorPos + textLength;
    }

    /* Store this operation in the Undo stack */
    if (push(&editor->undoStack, operation) == 0)
    {
        free(operation.text); //avoids memory leak
        return FAILURE;
    }

    /* A new operation invalidates all previous Redo operations */
    clearStack(&editor->redoStack);

    return SUCCESS;
}

/*
    Create a new line after the current line.
    A new node is added to the doubly linked list. The cursor then moves to the beginning of the new line.
*/
Status insertNewLine(TextEditor *editor)
{
    Node *newNode;
    Operation operation;

    /* A new line cannot be created without a current line. */
    if (editor->cursor == NULL)
    {
        return FAILURE;
    }

    /* Allocate memory for the new line node. */
    newNode = malloc(sizeof(Node));

    if (newNode == NULL)
    {
        return FAILURE;
    }

    /* Allocate memory for an empty string. */
    newNode->data = malloc(1);

    if (newNode->data == NULL)
    {
        free(newNode);
        return FAILURE;
    }

    newNode->data[0] = '\0';

    /* Store the New Line operation */
    operation.type = NEW_LINE;

    operation.text = NULL;
    operation.line = editor->cursorLine;
    operation.position = editor->cursorPos;

    /* Connect the new node after the current line */
    newNode->prev = editor->cursor;
    newNode->next = editor->cursor->next;

    /* If a next line already exists, connect it back to the new node */
    if (editor->cursor->next != NULL)
    {
        editor->cursor->next->prev = newNode;
    }
    else
    {
        /* The new node becomes the last line. */
        editor->tail = newNode;
    }

    /* Connect the current line to the new line. */
    editor->cursor->next = newNode;

    /* Move the cursor to the newly created line */
    editor->cursor = newNode;
    editor->cursorLine++;
    editor->cursorPos = 0;

    /* Store the new line operation in Undo */
    if(push(&editor->undoStack, operation) == 0)
    {
        /* If the operation cannot be stored, remove the newly created node */
        if(newNode->next != NULL)
        {
            newNode->next->prev = newNode->prev;
        }
        else
        {
            editor->tail = newNode->prev;
        }

        newNode->prev->next = newNode->next;

        editor->cursor = newNode->prev;
        editor->cursorLine--;
        editor->cursorPos = operation.position;

        free(newNode->data);
        free(newNode);

        return FAILURE;
    }

    /* New editing operation clears the Redo stack */
    clearStack(&editor->redoStack);

    return SUCCESS;
}

/*
    Delete the character at the current cursor position.
    The deleted character is stored in the Undo stack so that it can be restored later 
*/
Status deleteText(TextEditor *editor)
{
    Operation operation;

    int currentLength;
    int newLength;

    char *newData;

    /* Do not delete if the editor is empty */
    if(editor->cursor == NULL)
    {
        return FAILURE;
    }

    currentLength = strlen(editor->cursor->data);

    /* If the cursor is at the end of the line, there is no character to delete */
    if(editor->cursorPos >= currentLength)
    {
        return FAILURE;
    }

    /* Store the character that is going to be deleted. This information will be used by Undo */
    operation.type = DELETE;

    operation.text = malloc(2);

    if(operation.text == NULL)
    {
        return FAILURE;
    }

    operation.text[0] = editor->cursor->data[editor->cursorPos];
    operation.text[1] = '\0';

    operation.line = editor->cursorLine;
    operation.position = editor->cursorPos;

    newLength = currentLength - 1;

    /* Allocate memory for the new string after removing one character */
    newData = malloc(newLength + 1);

    if(newData == NULL)
    {
        free(operation.text);
        return FAILURE;
    }

    /* Copy the part before the character being  deleted */
    memcpy(newData, editor->cursor->data, editor->cursorPos);

    /* Copy the part after the character being deleted */
    memcpy(newData + editor->cursorPos, editor->cursor->data + editor->cursorPos + 1, currentLength - editor->cursorPos);

    /* Add the string termination */
    newData[newLength] = '\0';

    /* Replace the old string with the new string */
    free(editor->cursor->data);

    editor->cursor->data = newData;

    /* Store the delete operation in the Undo stack */
    if(push(&editor->undoStack, operation) == 0)
    {
        free(operation.text);
        return FAILURE;
    }

    /* A new editing operation clears the Redo stack */
    clearStack(&editor->redoStack);

    return SUCCESS;
}

/*
    Undo the most recent editing operation 
    INSERT operation: Remove the inserted text.
    DELETE operation: Restore the deleted text.
    The operation is moved from the Undo stack to the Redo stack 
*/
Status undo(TextEditor *editor)
{
    Operation operation;

    Node *current;

    int targetLine;

    int length;
    int textLength;
    int newLength;

    char *newData;

    /* Check whether there is any operation available for undo */
    if(isStackEmpty(&editor->undoStack))
    {
        return FAILURE;
    }

    /* Get the most recent operation */
    pop(&editor->undoStack, &operation);

    /* Find the line where the operation occured */
    targetLine = operation.line;

    current = editor->head;

    while(current != NULL && targetLine > 1)
    {
        current = current->next;
        targetLine--;
    }

    if(current == NULL)
    {
        return FAILURE;
    }

    /* Undo on INSERT operation. Remove the inserted text from the line */
    if(operation.type == INSERT)
    {
        length = strlen(current->data);
        textLength = strlen(operation.text);

        newLength = length - textLength;

        newData = malloc(newLength + 1);

        if(newData == NULL)
        {
            return FAILURE;
        }

        /* Copy text before the inserted text. */
        memcpy(newData, current->data, operation.position);

        /* Copy text after the inserted text. */
        memcpy(newData + operation.position, current->data + operation.position + textLength, length - operation.position - textLength);

        newData[newLength] = '\0';

        free(current->data);
        current->data = newData;

        /* Restore the cursor position to where the insertion originally happened */
        editor->cursor = current;
        editor->cursorLine = operation.line;
        editor->cursorPos = operation.position;
    }
    /* Undo a DELETE operation. Insert the deleted text back into the line */
    else if (operation.type == DELETE)
    {
        length = strlen(current->data);
        textLength = strlen(operation.text);

        newLength = length + textLength;

        newData = malloc(newLength + 1);

        if (newData == NULL)
        {
            return FAILURE;
        }

        /* Copy text before the deleted character. */
        memcpy(newData, current->data, operation.position);

        /* Restore the deleted text. */
        memcpy(newData + operation.position, operation.text, textLength);

        /* Copy the remaining text. */
        memcpy(newData + operation.position + textLength, current->data + operation.position, length - operation.position);

        newData[newLength] = '\0';

        free(current->data);
        current->data = newData;

        /* Restore the cursor position */
        editor->cursor = current;
        editor->cursorLine = operation.line;
        editor->cursorPos = operation.position;
    }
    /* Undo the new line */
    else if(operation.type == NEW_LINE)
    {
        /*  The new line was created immediately after the line stored in operation.line 
            current points to the line before the newly created line
        */
        Node *lineToDelete = current->next;

        if(lineToDelete == NULL)
        {
            return FAILURE;
        }

        /* Connect the previous line directly to the line after the newly created line */
        current->next = lineToDelete->next;

        if(lineToDelete->next != NULL)
        {
            lineToDelete->next->prev = current;
        }
        else
        {
            /* The previous line becomes the tail */
            editor->tail = current;
        }

        /* Move the cursor back to the line where the new line was created */
        editor->cursor = current;
        editor->cursorLine = operation.line;
        editor->cursorPos = operation.position;

        free(lineToDelete->data);
        free(lineToDelete);
    }

    /* Move the operation to the Redo stack. The text pointer is transferred to the Redo stack, so we must NOT free operation.text here */
    if (push(&editor->redoStack, operation) == 0)
    {
        free(operation.text);
        return FAILURE;
    }

    return SUCCESS;
}

/*
    Redo the most recently undone operation.
    INSERT operation: Insert the text again.
    DELETE operation: Delete the text again.
    The operation is moved from the Redo stack back to the Undo stack.
*/
Status redo(TextEditor *editor)
{
    Operation operation;

    Node *current;

    int targetLine;

    int length;
    int textLength;
    int newLength;

    char *newData;

    /* Check whether there is any operation available for Redo */
    if (isStackEmpty(&editor->redoStack))
    {
        return FAILURE;
    }

    /* Get the most recently undone operation. */
    pop(&editor->redoStack, &operation);

    /* Find the line where the operation occurred */
    targetLine = operation.line;

    current = editor->head;

    while (current != NULL && targetLine > 1)
    {
        current = current->next;
        targetLine--;
    }

    if (current == NULL)
    {
        return FAILURE;
    }

    /* Redo an INSERT operation. Insert the text again */
    if (operation.type == INSERT)
    {
        length = strlen(current->data);
        textLength = strlen(operation.text);

        newLength = length + textLength;

        newData = malloc(newLength + 1);

        if (newData == NULL)
        {
            return FAILURE;
        }

        /* Copy text before the insertion position. */
        memcpy(newData, current->data, operation.position);

        /* Insert the text again. */
        memcpy(newData + operation.position, operation.text, textLength);

        /* Copy the remaining text. */
        memcpy(newData + operation.position + textLength, current->data + operation.position, length - operation.position);

        newData[newLength] = '\0';

        free(current->data);
        current->data = newData;

        /* Restore the cursor position to after the inserted text */
        editor->cursor = current;
        editor->cursorLine = operation.line;
        editor->cursorPos = operation.position + textLength;
    }
    /* Redo a DELETE operation. Delete the same text again */
    else if (operation.type == DELETE)
    {
        length = strlen(current->data);
        textLength = strlen(operation.text);

        newLength = length - textLength;

        newData = malloc(newLength + 1);

        if (newData == NULL)
        {
            return FAILURE;
        }

        /* Copy text before the deleted text. */
        memcpy(newData, current->data, operation.position);

        /* Copy text after the deleted text. */
        memcpy(newData + operation.position, current->data + operation.position + textLength, length - operation.position - textLength);

        newData[newLength] = '\0';

        free(current->data);
        current->data = newData;

        /* Keep the cursor at the deletion position */
        editor->cursor = current;
        editor->cursorLine = operation.line;
        editor->cursorPos = operation.position;
    }
    /* Redo the new line creation */
    else if(operation.type == NEW_LINE)
    {
        Node *newNode;

        /* Find the line after which a new line should be created */
        newNode = malloc(sizeof(Node));

        if(newNode == NULL)
        {
            return FAILURE;
        }

        newNode->data = malloc(1);
        if(newNode->data == NULL)
        {
            free(newNode);
            return FAILURE;
        }

        newNode->data[0] = '\0';

        /* Insert the new node after the current line */
        newNode->prev = current;
        newNode->next = current->next;

        if(current->next != NULL)
        {
            current->next->prev = newNode;
        }
        else
        {
            editor->tail = newNode;
        }

        current->next = newNode;

        /* Move the cursor to the newly recreated line */
        editor->cursor = newNode;
        editor->cursorLine = operation.line + 1;
        editor->cursorPos = 0;
    }

    /* Move the operation back to the Undo stack. The text pointer is transferred to the Undo stack */
    if (push(&editor->undoStack, operation) == 0)
    {
        free(operation.text);
        return FAILURE;
    }

    return SUCCESS;
}

/* Display all the text stored in the editor. Each node of the doubly linked list represents one line of text */
void displayText(TextEditor *editor)
{
    Node *current;
    int lineNumber = 1;

    /* Check whether the editor contains any text */
    if(editor->head == NULL)
    {
        printf("\n\033[31mEditor is empty\033[0m\n");
        return;
    }

    printf("\n\033[34m================Text================\033[0m\n");

    current = editor->head;

    /* Traverse the linked list from the first line until the last line */
    while(current != NULL)
    {
        printf("%d. %s\n", lineNumber, current->data);

        current = current->next;
        lineNumber++;
    }

    printf("\033[34m====================================\033[0m\n");

    /* Display the current cursor position */
    printf("Cursor: Line %d, Position %d\n", editor->cursorLine, editor->cursorPos);
}

/*
    Move the cursor one position to the left.
    The cursor cannot move before the beginning of the current line.
*/
void moveCursorLeft(TextEditor *editor)
{
    /* Do nothing if the editor is empty */
    if(editor->cursor == NULL)
    {
        return;
    }

    if(editor->cursorPos > 0)
    {
        editor->cursorPos--;
    }
}

/*
    Move the cursor one position to the right.
    The cursor cannot move beyond the end of the current line.
*/
void moveCursorRight(TextEditor *editor)
{
    int length;

    /* Do nothing if the editor is empty */
    if(editor->cursor == NULL)
    {
        return;
    }

    length = strlen(editor->cursor->data);

    if(editor->cursorPos < length)
    {
        editor->cursorPos++;
    }
}

/*
    Move the cursor to the previous line.
    If the previous line is shorter than the current cursor position, place the cursor at its end.
*/
void moveCursorUp(TextEditor *editor)
{
    int length;

    /* Do nothing if the editor is empty */
    if(editor->cursor == NULL)
    {
        return;
    }

    /* Check whether a previous line exists */
    if(editor->cursor->prev != NULL)
    {
        editor->cursor = editor->cursor->prev;
        editor->cursorLine--;

        length = strlen(editor->cursor->data);

        /* Keep the cursor within the length of the new line */
        if(editor->cursorPos > length)
        {
            editor->cursorPos = length;
        }
    }
}

/*
    Move the cursor to the next line.
    If the next line is shorter than the current cursor position, place the cursor at its end 
*/
void moveCursorDown(TextEditor *editor)
{
    int length;

    /* Do nothing if the editor is empty */
    if(editor->cursor == NULL)
    {
        return;
    }

    /* Check whether a next line exists */
    if(editor->cursor->next != NULL)
    {
        editor->cursor = editor->cursor->next;
        editor->cursorLine++;

        length = strlen(editor->cursor->data);

        /* Keep the cursor within the length of the new line */
        if(editor->cursorPos > length)
        {
            editor->cursorPos = length;
        }
    }
}