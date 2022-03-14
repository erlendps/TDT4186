#include "sem.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

SEM *sem;

void *print_number(void *number);

int main(void) {
  sem = sem_init(1);
  int pid = getpid();
  
  int iret1;
  pthread_t threads[32];
  int susses[32];
  for (int i = 0; i < 32; i++) {
    P(sem);
    susses[i] = i;
    iret1 = pthread_create(&threads[i], NULL, print_number, (void*) &susses[i]);
  }
  usleep(1000);
  return 0;
}


void *print_number(void *number) {
  int pid = getpid();
  printf("\nPID: %i\n", pid);
  printf("Number: %i\n", *(int*)number);
  V(sem);
  return 0;
}
