#include "main.h"

void execute_internal_commands(char *input_string)
{
    char *command = get_command(input_string);

    /* Execute jobs command */
    if(strcmp(command , "jobs") == 0)
    {
        print_jobs();
    }

    /* Execute exit command */
    else if(strcmp(command, "exit") == 0)
    {
        exit(0);
    }

    /* Execute pwd command */
    else if(strcmp(command, "pwd") == 0)
    {
        char cwd[100];

        if(getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
            exit_status = 0;
        }
        else
        {
            perror("pwd");
            exit_status = 1;
        }
    }

    /* Execute cd command */
    else if(strcmp(command , "cd") == 0)
    {
        if(chdir(input_string + 3) != 0)
        {
            perror("cd");
            exit_status = 1;
        }
        else
        {
            exit_status = 0;
        }
    }
    /* Execute echo command */
    else if(strncmp(command , "echo", 4) == 0)
    {
        echo(input_string, exit_status);
    }
    /* Execute fg command */
    else if(strcmp(command, "fg") == 0)
    {
        if(head == NULL)
        {
            printf("-bash: fg: current: no such job\n");
        }
        else
        {
            int status;
            Job *temp = head;
            Job *prev = NULL;

            int job_no = 0;

            /* fg without number -> latest job */
            if(strcmp(input_string, "fg") == 0)
            {
                temp = head;
            }
            else
            {
                /* Get requested job number */
                job_no = atoi(input_string + 3);

                /* Find the job using its actual job number */
                temp = head;

                while(temp != NULL)
                {
                    if(temp->job_no == job_no)
                    {
                        break;
                    }

                    temp = temp->link;
                }

                if(temp == NULL)
                {
                    printf("-bash: fg: current: no such job\n");
                    return;
                }
            }

            pid_t fg_pid = temp->pid;

            /* Print command */
            printf("%s\n", temp->command);
            fflush(stdout);

            /* Continue process */
            kill(fg_pid, SIGCONT);

            pid = fg_pid;

            /* Wait for foreground process */
            waitpid(fg_pid, &status, WUNTRACED);

            if(WIFSTOPPED(status))
            {
                temp->status = STOPPED;
            }
            else if(WIFEXITED(status) || WIFSIGNALED(status))
            {
                /* Remove this particular job */
                if(temp == head)
                {
                    delete_at_first();
                }
                else
                {
                    prev = head;

                    while(prev->link != temp)
                    {
                        prev = prev->link;
                    }

                    prev->link = temp->link;

                    free(temp);
                }
            }

            pid = 0;
        }
    }
    /* Execute bg command */
    else if(strcmp(command, "bg") == 0)
    {
        if(head == NULL)
        {
            printf("-bash: bg: current: no such job\n");
        }
        else
        {
            printf("[%d]+ %s &\n", head->job_no, head->command);

            kill(head->pid, SIGCONT);

            head->status = RUNNING;
        }
    }
}

void echo(char *input_string, int status)
{
    /* echo with no argument */
    if(strcmp(input_string, "echo") == 0)
    {
        printf("\n");
    }
    /* echo $SHELL */
    else if(strcmp(input_string, "echo $SHELL") == 0)
    {
       printf("%s\n", getenv("SHELL"));
    }
    /* echo $$ */
    else if(strcmp(input_string, "echo $$") == 0)
    {
        printf("%d\n", getpid());
    }
    else if(strcmp(input_string, "echo $?") == 0)
    {
        printf("%d\n", status);
    }
    /* Normal echo */
    else
    {
        printf("%s\n", input_string + 5);
    }

}
