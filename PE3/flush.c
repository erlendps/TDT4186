//#include <sys/socket.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define STR_EQUAL 0
#define MAX_ARGS 16
typedef struct {
    size_t arg_count;
    char name[255];
    char* args[MAX_ARGS];
    bool background_task;
} command_t;

typedef struct linked_process {
    char name[255];
    pid_t pid;
    struct linked_process* next_process;
} linked_process_t;

void parse_command(command_t *arg_output, char* command);
pid_t execute_command(command_t *command);

void insert_process(linked_process_t *head, char name[255], pid_t pid) {
    linked_process_t *tail = head;
    while (tail->next_process != NULL) {
        tail = tail->next_process;
    }
    
    tail->next_process = malloc(sizeof(linked_process_t));
    linked_process_t *process = tail->next_process;
    strcpy(process->name, name);
    process->pid = pid;
    process->next_process = NULL;
}

bool delete_process(linked_process_t *head, pid_t pid) {
    linked_process_t *process = head;
    linked_process_t *process_prev = process;
    
    while (process != NULL) {
        if (process->pid == pid) {
            process_prev->next_process = process->next_process;
            free(process);
            return true;
        }
        process_prev = process;
        process = process->next_process;
    }

    return false;
}

void print_processes(linked_process_t *head) {
    linked_process_t *process = head->next_process;
    printf("PID     Name\n");
    printf("------------\n");
    while (process != NULL) {
        printf("%05i   %s\n", process->pid, process->name);
        process = process->next_process;
    }
}

void parse_command(command_t *arg_output, char* command) {
    int i = 0; // counts argument
    int j = 0; // counts letter in argument
    int h = 0; // counts letter in command
    char x = command[0]; // holds char from command
    arg_output->args[0] = malloc(255);
    while (x != '\0') {
        x = command[h];
        if ((int) x == 9 || x == ' ') {
            arg_output->args[i][j] = '\0';
            i++;
            j = -1;
            arg_output->args[i] = malloc(255);
        } else {
            arg_output->args[i][j] = x;
        }
        j++;
        h++;
    }
    // decrement to replace \n with \0
    j--; 
    arg_output->args[i][j] = '\0';

    // increment to set last pointer to NULL
    i++;
    arg_output->args[i] = NULL;

    // set arg count
    arg_output->arg_count = i;

    // set background task 
    if (arg_output->args[i-1][j-1] == '&') {
        if (j == 1) {
            arg_output->args[i-1] = NULL;
        } else {
            arg_output->args[i-1][j-1] = '\0';
        }

        arg_output->background_task = true;
    } else {
        arg_output->background_task = false;
    }
    strcpy(arg_output->name, arg_output->args[0]);
}

pid_t execute_command(command_t *command) {
    pid_t pid;

    bool in = false, out = false;
    char input[64], output[64];
    int i = 0;
    while (command->args[i] != NULL) {
        if (strcmp(command->args[i], "<") == STR_EQUAL) {
            command->args[i] = NULL;
            strcpy(input, command->args[i + 1]);
            in = true;
        }
        else if(strcmp(command->args[i], ">") == STR_EQUAL) {      
            command->args[i] = NULL;
            strcpy(output, command->args[i + 1]);
            out = true;
        } 
        i++;
    }

    pid = fork();
    if (pid == 0) {

        if (in) {
            int fd0 = open(input, O_RDONLY, 0);
            if (fd0 < 0) {
                exit(errno);
            }
            if (dup2(fd0, STDIN_FILENO) < 0) {
                close(fd0);
                exit(errno);
            }
            close(fd0);
        }

        if (out) {
            int fd1 = creat(output , 0644);
            if (dup2(fd1, STDOUT_FILENO) < 0) {
                close(fd1);
                exit(errno);
            }
            close(fd1);
        }
    
        int e = execvp(command->name, &(command->args[0]));
        if (e == -1) {
            printf("Command failed.\n");
            exit(errno); // Exit status
        }

        return 0;
    } else {
        return pid;
    }
}


int main() {
    pid_t pid;
    int exit_status = 0;
    int status = 0;
    command_t command;
    char input[256];
    pid_t zombie;
    linked_process_t head;
    strcpy(head.name, "head");
    head.next_process = NULL;
    head.pid = 0;

    while (1) {
        exit_status = 0;
        char path[256];

        // flush should collect all background processes that have terminated (zombies) and print
        // their exit status like for the foreground processes of the first subtask.
        
        do {
            zombie = waitpid(0, &status, WNOHANG);
            if (zombie > 0) {
                delete_process(&head, zombie);
            }
        } while (zombie > 0);

        getcwd(path, sizeof(path));
        printf("\n%s : ", path);
        if (fgets(&input[0], 256, stdin) == NULL) {
            return 0;
        }
        input[strlen(input)-1] = '\0';
        parse_command(&command, input);

        if (strcmp("cd", command.name) == STR_EQUAL) {
            int res = chdir((command.args[1]));
            if (res == -1) {
                if (errno == 14) {
                    chdir(getenv("HOME"));
                } else {
                    exit_status = errno;
                }
            }
        } else if (strcmp("jobs", command.name) == STR_EQUAL) {
            print_processes(&head);
        } else {
            pid = execute_command(&command);
            if (command.background_task) {
                insert_process(&head, command.name, pid);
                printf("Started command [%s] in the background\n", input);
            } else {
                waitpid(pid, &status, 0);
                exit_status = WEXITSTATUS(status);
            }
        }
        printf("\nExit status [%s] = %i\n", input, exit_status);
        
        // reset command struct
        for (int i = 0; i < command.arg_count; i++) {
            free(command.args[i]);
        }
        strcpy(command.name, "");
    }
    return 0;
}
