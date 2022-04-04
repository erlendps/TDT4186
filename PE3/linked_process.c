#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct linked_process {
    char name[255];
    pid_t pid;
    struct linked_process* next_process;
} linked_process_t;

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
