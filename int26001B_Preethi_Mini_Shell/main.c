#include "main.h"

/* Global variables used for process and  job management */
char prompt[50] = "minishell$";
char *external_commands[200];
pid_t pid = 0;
Job *head = NULL;
char current_command[100];
int child_changed = 0;
int current_job_no = 0;
int previous_job_no = 0;
int next_job_no = 1;
int exit_status = 0;

int main()
{
    /* clear the visible screen terminal before starting the shell */
    system ("clear");

    char input_string[100];

    /* Load the list of available external commands */
    extract_external_commands(external_commands);

    /* Start the shell input and command execution loop */
    scan_input(prompt, input_string);

    return 0;
}

