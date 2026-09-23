/*
    PROJECT NAME : TEXT EDITOR

    DESCRIPTION : This project implements a console-based Text Editor in C using a doubly linked list to manage text lines.

    Features implemented in this stage:
    - Text insertion and deletion
    - Cursor movement
    - Undo and Redo
    - Text display
    - Dynamic memory management
*/


#include "text_editor.h"

int main()
{
    TextEditor editor;
    int choice;

    char text[1000];

    /* Initialize the editor and its required data structure */
    initEditor(&editor);

    while(1)
    {
        printf("\n\033[34m==================TEXT EDITOR==================\033[0m\n");

        printf("1. Insert Text\n");
        printf("2. Delete Text\n");
        printf("3. Move Cursor Up\n");
        printf("4. Move Cursor Down\n");
        printf("5. Move Cursor Left\n");
        printf("6. Move Cursor Right\n");
        printf("7. Undo\n");
        printf("8. Redo\n");
        printf("9. Display Text\n");
        printf("10. Exit\n");

        printf("\n\033[34m===============================================\033[0m\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch(choice)
        {
            /* Insert the text at the current sursor position */
            case 1:
            {
                printf("Enter the text: ");
                fgets(text, sizeof(text), stdin);

                /* Remove the newline added by fgets() */
                text[strcspn(text, "\n")] = '\0';

                /* If the user presses Enter without entering text, create a new line */
                if(text[0] == '\0')
                {
                    if(insertNewLine(&editor) == SUCCESS)
                        printf("\033[32mNew line created successfully\033[0m\n");
                    else
                        printf("\033[32mNew line creation failed\033[0m\n");
                }
                else
                {
                    if(insertText(&editor, text) == SUCCESS)
                        printf("\033[32mText inserted successfully\033[0m\n");
                    else
                        printf("\033[31mInsertion Failed\033[0m\n");
                }

                break;
            }

            /* Delete the text at the current cursor position */
            case 2:
            {
                if(deleteText(&editor) == SUCCESS)
                    printf("\033[32mText deleted successfully\033[0m\n");
                else
                    printf("\033[31mNothing to delete\033[0m\n");

                break;
            }

            /* Move cursor to the previous line */
            case 3:
            {
                moveCursorUp(&editor);
                break;
            }

            /* Move cursor to the next line */
            case 4:
            {
                moveCursorDown(&editor);
                break;
            }

            /* Move cursor one position to the left */
            case 5:
            {
                moveCursorLeft(&editor);
                break;
            }

            /* Move cursor one position to the right */
            case 6:
            {
                moveCursorRight(&editor);
                break;
            }

            /* Undo the most recent editing opearation */
            case 7:
            {
                if(undo(&editor) == SUCCESS)
                    printf("\033[32mUndo Successfull\033[0m\n");
                else
                    printf("\033[31mNothing to undo\033[0m\n");
                
                break;
            }

            /* Redo the most recently undone operation */
            case 8:
            {
                if(redo(&editor) == SUCCESS)
                    printf("\033[32mRedo Successfull\033[0m\n");
                else
                    printf("\033[31mNothing to redo\033[31m\n");

                break;
            }

            /* Display the complete text */
            case 9:
            {
                displayText(&editor);
                break;
            }

            /* Free all dynamically alloacated memory and exit */
            case 10:
            {
                printf("\033[31mExiting editor.....\033[0m\n");
                freeEditor(&editor);

                return 0;
            }

            default:
                printf("\033[31mInvalid choice!\n");
        }
    }

    return 0;
}