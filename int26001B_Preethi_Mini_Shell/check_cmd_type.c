#include "main.h"

int check_command_type(char *command)
{
    //either external or internal

    //List of builtin commands
	char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", "jobs", "fg", "bg", NULL};

    /* Check whether the command is a built-in command */
    for(int i=0; builtins[i] != NULL; i++)
    {
        if(strcmp(command, builtins[i]) == 0)
        {
            return BUILTIN;
        }
    }

    /* Check whether the command is an external command */
    for(int i=0; external_commands[i] != NULL; i++)
    {
        if(strcmp(command, external_commands[i]) == 0)
        {
            return EXTERNAL;
        }
    }

    /* Command is neither built-in nor external */
    return NO_COMMAND;
}

void extract_external_commands(char **external_commands)
{
    FILE *fp;
    char command[50];
    int i = 0;

    // open the externalscmds.txt
    fp = fopen("ext_cmds.txt", "r");
    
    if(fp == NULL)
    {
        perror("fopen");
        return;
    }


    /*read external commands one by one */
    while(fscanf(fp, "%49s", command) == 1)
    {
        /*allocate memory dynamically for each command */
        external_commands[i] = malloc(strlen(command) + 1);

        if(external_commands[i] == NULL)
        {
            printf(ANSI_COLOR_RED "Memory allocation failed\n" ANSI_COLOR_RESET);
            fclose(fp);
            return;
        }

        /* store the cmd into the array */
        strcpy(external_commands[i], command);
        i++;
    }

    /* Mark the end of external_command array */
    external_commands[i] = NULL;

    fclose(fp);
}

