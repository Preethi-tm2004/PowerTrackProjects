#include "main.h"

void execute_external_commands(char *input_string)
{
    char *args[20];
    int i=0;
    int status;

    char *token = strtok(input_string, " ");

    /* Tokenize the input string into command arguments */
    while(token != NULL)
    {
        args[i] = token;
        i++;

        token = strtok(NULL, " ");
    }

    args[i] = NULL;

    /* Create a child process to execute the external command */
    pid = fork();

    if(pid < 0)
    {
        perror("fork");
    }
    else if(pid == 0)
    {
        /* Restore the default signal handling for the child process */
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        /* Replace the child process with the requested command */
        execvp(args[0], args);

        perror("execvp");
        exit(127);
    }
    else
    {
        /* Wait for the child process and detect stopped store */
        waitpid(pid, &status, WUNTRACED);

        if(WIFSIGNALED(status))
        {
            printf("\n");
        }

        /* Store the exit status of the child process */
        if(WIFEXITED(status))
        {
            exit_status = WEXITSTATUS(status);
        }
        else if(WIFSIGNALED(status))
        {
            exit_status = 128 + WTERMSIG(status);
        }

        fflush(stdout);
        pid = 0;
    }
}