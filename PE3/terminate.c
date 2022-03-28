#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include<signal.h>

pid_t cpid;

void sig_handler_child(int sig_num) {
    printf("\ndskgnlfsjg\n");
    kill(getpid(), SIGINT);
    printf("\nInside child handler\n");
}

void* scan_for_eof(void* arg) {
    int pid = *((int *) arg);
    //free(arg);
    while (fgetc(stdin) != -1);

    kill(cpid, SIGUSR1);
    // while (1) {
    //   if (fgetc(stdin) == -1) {
    //     printf("\ninside scan_for_eof\n");
    //     kill(cpid, SIGTERM);
    //   }
    // }
    return 0;
}


int main() {

    pid_t pid = fork();

    if (pid < 0) exit(1);

    if (pid == 0) {
      signal(SIGUSR1, sig_handler_child);
      printf("\nDoing shit rn\n");
      execlp("/usr/bin/python3", "python", "test.py", NULL);
      printf("\n");
    }
    else {
      printf("\nmain\n");
      cpid = pid;
      pthread_t t1;
      pthread_create(&t1, NULL, scan_for_eof, (void*) &pid);
      pthread_join(t1, NULL);
    }
    while(1) {
      printf("\noutside\n");
      sleep(1);
    }
    return 0;
}