#include "main.h"

void execute_n_pipe(char *input_string)
{
    int pipe_count = 0;
    int cmd_count;

    /* Count the number of pipes */
    for(int i=0; input_string[i] != '\0'; i++)
    {
        if(input_string[i] == '|')
        {
            pipe_count++;
        }
    }

    /* Number of commands is one more than number of pipes */
    cmd_count = pipe_count + 1;

    /* Dynamically allocate command array */
    char **commands = malloc((cmd_count + 1) * sizeof(char *));

    if(commands == NULL)
    {
        perror("malloc");
        return;
    }

    /* Dynamically allocate pipes */
    int (*pipes)[2] = malloc(pipe_count * sizeof(int[2]));

    if(pipes == NULL)
    {
        perror("malloc");
        free(commands);
        return;
    }

    /* Dynamically allocate process ID's */
    pid_t *child_pid = malloc(cmd_count * sizeof(pid_t));

    if(child_pid == NULL)
    {
        perror("malloc");
        free(commands);
        free(pipes);
        return;
    }

    /* Split input into individual commands */
    int i =0;

    commands[i] = strtok(input_string, "|");

    while(commands[i] != NULL)
    {
        i++;
        commands[i] = strtok(NULL, "|");
    }

    /* Create pipes */
    for(int i=0; i < pipe_count; i++)
    {
        if(pipe(pipes[i]) == -1)
        {
            perror("pipe");

            free(commands);
            free(pipes);
            free(child_pid);

            return;
        }
    }

    /* Create child processes */
    for(int i = 0; i < cmd_count; i++)
    {
        child_pid[i] = fork();

        if(child_pid[i] < 0)
        {
            perror("fork");

            free(commands);
            free(pipes);
            free(child_pid);

            return;
        }

        if(child_pid[i] == 0)
        {
            /* Child should use default signal behaviour */
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            char *args[20]; //number of arguments in one command 
            int j =0;

            /* Take input from previous pipe */
            if(i > 0)
            {
                if(dup2(pipes[i-1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    _exit(1);
                }

            }

            /* Send output to next pipe */
            if(i < cmd_count -1 )
            {
                if(dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    _exit(1);
                }
            }

            /* Close all pipe descriptors */
            for(int k = 0; k < pipe_count; k++)
            {
                close(pipes[k][0]);
                close(pipes[k][1]);
            }

            /* Convert command into argument array */
            char *token = strtok(commands[i], " \t");

            while(token != NULL)
            {
                args[j++] = token;
                token = strtok(NULL, " \t");
            }

            args[j] = NULL;

            /* Execute command */
            execvp(args[0], args);

            perror("execvp");

            _exit(1);
        }
    }

    /* Parent closes all pipe descriptors */
    for(int i=0; i < pipe_count; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int status;
    /* Wait for all children */
    for(i = 0; i < cmd_count; i++)
    {
        waitpid(child_pid[i], &status, WUNTRACED);
    }

    /* Free dynamically allocated memory */
    free(commands);
    free(pipes);
    free(child_pid);

}