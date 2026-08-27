#include "main.h"

/* insert a new job at the begining of the new job */
void insert_at_first(pid_t pid, char *command)
{
    Job *new = malloc(sizeof(Job));

    if(new == NULL)
    {
        perror("malloc");
        return;
    }

    new->pid = pid;
    strcpy(new->command, command);

    new->status = STOPPED;
    new->job_no = next_job_no++;

    /* Update the current and previous job numbers */
    previous_job_no = current_job_no;
    current_job_no = new->job_no;

    new->link = head;
    head = new;
}

void print_jobs(void)
{
    /* Update the status of completed background jobs */
    check_background_jobs();

    /* Print the jobs in the order they were created */
    print_jobs_reverse(head);
}

void print_jobs_reverse(Job *temp)
{
    if(temp == NULL)
    {
        return;
    }

    print_jobs_reverse(temp->link);

    /* Display stopped jobs with current/previous job markers */
    if(temp->status == STOPPED)
    {
        if(temp->job_no == current_job_no)
        {
            printf("[%d]+   Stopped             %s\n", temp->job_no, temp->command);
        }
        else if(temp->job_no == previous_job_no)
        {
            printf("[%d]-   Stopped             %s\n", temp->job_no, temp->command);
        }
        else
        {
            printf("[%d]    Stopped             %s\n", temp->job_no, temp->command);
        }
    }
    /* Display the running background jobs with current/ previous markers */
    else if(temp->status == RUNNING)
    {
        if(temp->job_no == current_job_no)
        {
            printf("[%d]+   Running             %s &\n",temp->job_no, temp->command);
        }
        else if(temp->job_no == previous_job_no)
        {
            printf("[%d]-   Running             %s &\n",temp->job_no, temp->command);
        }
        else
        {
            printf("[%d]    Running             %s &\n",temp->job_no, temp->command);
        }
    }
}

/* delete the command in the list */
void delete_at_first(void)
{
    if(head == NULL)
    {
        return;
    }

    Job *temp = head;

    head = head->link;

    free(temp);

}

void check_background_jobs(void)
{
    Job *temp = head;
    Job *prev = NULL;
    
    while(temp != NULL)
    {
        if(temp->status == RUNNING)
        {
            int status;

            /* Check whether the background process has completed */
            pid_t ret = waitpid(temp->pid, &status, WNOHANG);

            if(ret == temp->pid)
            {
                if(WIFEXITED(status) || WIFSIGNALED(status))
                {
                    printf("[%d]+   Done                %s\n", temp->job_no, temp->command);

                    /* Remove the completed Job from the linked list */
                    if(prev ==  NULL)
                    {
                        head = temp->link;
                    }
                    else
                    {
                        prev->link = temp->link;
                    }

                    free(temp);

                    return;
                }
            }
        }

        prev = temp;
        temp = temp->link;
    }
}

