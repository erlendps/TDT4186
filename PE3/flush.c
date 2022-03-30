#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <signal.h>
// bip bop
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define STR_EQUAL 0
#define MAX_ARGS 16

// We drew the conclusion that since _t means type, it would be appropriate
// to call this typedef command_t.
typedef struct {
    size_t arg_count;
    char name[255];
    char* args[MAX_ARGS];
    bool background_task;
} command_t;

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
    j++;
    arg_output->args[i][j] = '\0';
    i++;
    arg_output->args[i] = NULL;
    arg_output->arg_count = i;
    strcpy(arg_output->name, arg_output->args[0]);
    //printf("\n%s, %s\n", arg_output->args[i-1], x);

    if (arg_output->args[i-1][j-1] == '&') {
        arg_output->background_task = true;
    }
}

pid_t execute_command(command_t *command) {
    pid_t pid;

    int in = 0, out = 0;
    char input[64], output[64];
    int i = 0;
    while (command->args[i] != NULL) {
        if (strcmp(command->args[i], "<") == STR_EQUAL) {
            command->args[i] = NULL;
            strcpy(input, command->args[i + 1]);
            in = 1;
        }
        else if(strcmp(command->args[i], ">") == STR_EQUAL) {      
            command->args[i] = NULL;
            printf("\n!!\n");
            strcpy(output, command->args[i + 1]);
            out = 1;
        } 
        i++;
    }

    pid = fork();
    if (pid == 0) {

        if (in) {
            int fd0 = open(input, O_RDONLY);
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
        
        int e = execv(command->name, &(command->args[0]));
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
    pid_t child;
    int exit_status = 0;
    int status = 0;
    command_t command;
    char input[256];
    while (1) {
        exit_status = 0;
        pthread_t thread;
        char path[256];
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

        } else {
            child = execute_command(&command);
            waitpid(child, &status, 0);
            exit_status = WEXITSTATUS(status);
        }
        printf("Exit status [%s] = %i\n", input, exit_status);
        //printf("\n%d\n", command.background_task);
        for (int i = 0; i < command.arg_count; i++) {
            free(command.args[i]);
        }
        
    }
    return 0;
}