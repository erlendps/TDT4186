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

int filedes[2];

typedef struct {
    size_t number_args;
    char* args[16];
} command_args;

void parse_command(command_args *arg_output, char* command) {
    int i = 0; // counts argument
    int j = 0; // counts letter in argument
    int h = 0; // counts letter in command
    char x = command[0]; // holds char from command
    arg_output->args[0] = malloc(255);
    while (x != '\0') {
        x = command[h];
        if ((int)x == 9 || x == ' ') {
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
    arg_output->number_args = i;
}

pid_t execute_command(char** command) {
    pid_t child;

    child = fork();
    if (child == 0) {
        int e = execv(command[0], &command[0]);
        if (e == -1) {
            printf("Command failed.\n");
            exit(errno);
        }
    } else {
        return child;
    }
}

void* scan_for_eof(void* arg) {
    int pid = *((int *) arg);
    while (fgets(NULL, 100, stdin) != NULL) {;}
    printf("HSDBHAJSDN");
    kill(pid, SIGINT);
    return 0;
}


int main() {
    pid_t child;
    int status = 0;
    command_args command;
    char input[256];
    //pthread_t t1;
    while (1) {
        pthread_t thread;
        char path[256];
        getcwd(path, sizeof(path));
        printf("\n%s : ", path);
        if (fgets(&input[0], 256, stdin) == NULL) {
            return 0;
        }
        input[strlen(input)-1] = '\0';
        parse_command(&command, input);
        child = execute_command(command.args);
        waitpid(child, &status, 0);
        printf("Exit status [%s] = %i\n", input, WEXITSTATUS(status));
        for (int i = 0; i < command.number_args; i++) {
            free(command.args[i]);
        }
    }
    //pthread_create(&t1, NULL, scan_for_eof, NULL);
    //pthread_join(t1, NULL);
    return 0;
}