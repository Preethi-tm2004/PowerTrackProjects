#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "undo_redo.h"

/* Return status used by the editor operations */
typedef enum
{
    FAILURE,
    SUCCESS
}Status;

/* Each node represents one line of text */
typedef struct Node 
{
    char *data;
    struct Node *prev; // points to previous line 
    struct Node *next; // points to next line

}Node;

/* TextEditor stores complete state of the editor */
typedef struct
{
    Node *head; //first line
    Node *tail; //last line

    Node *cursor; //line where the cursor is currently present

    int cursorLine; //current line number
    int cursorPos; //cursor position within the current line 

    /* Stacks used for multi-level undo and redo */
    Stack undoStack;
    Stack redoStack;

}TextEditor;

/* Editor initialization and memory cleanup */
void initEditor(TextEditor *editor);
void freeEditor(TextEditor *editor);

/* Text editing operation */
Status insertText(TextEditor *editor, const char *text);
Status deleteText(TextEditor *editor);

/* Create a new line at the current cursor position */
Status insertNewLine(TextEditor *editor);

/* Cursor movement operations. */
void moveCursorUp(TextEditor *editor);
void moveCursorDown(TextEditor *editor);
void moveCursorLeft(TextEditor *editor);
void moveCursorRight(TextEditor *editor);

/* Display operations. */
void displayText(TextEditor *editor);

/* Undo and Redo operations. */
Status undo(TextEditor *editor);
Status redo(TextEditor *editor);

#endif