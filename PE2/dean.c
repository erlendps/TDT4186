#include "sem.h"
#include "bbuffer.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#define BUFFERSIZE 32
#define CLIENTS 16
#define THREADS 1

SEM *sem;
BNDBUF *buffer;
void *receive_request(void *number);
void *send_request(void *message);

int main(void) {
  int thread;
  sem = sem_init(1);
  // Buffer size 32
  buffer = bb_init(BUFFERSIZE);

  // 8 receiving threads
  pthread_t server_threads[THREADS];
  int numbers[THREADS];
  for (int i = 0; i < THREADS; i++) {
    numbers[i] = i;
    thread = pthread_create(&server_threads[i], NULL, receive_request, (void*) &numbers[i]);
  }

  // 2048 clients
  pthread_t client_threads[CLIENTS];
  int client_messages[CLIENTS];
  for (int i = 0; i < CLIENTS; i++) {
    client_messages[i] = i;
    thread = pthread_create(&client_threads[i], NULL, send_request, (void*) &client_messages[i]);
    usleep(100);
  }

  while (1);
  return 0;
}

void *send_request(void *message) {
  time_t before;
  while (1) {
    time(&before);
    bb_add(buffer, (long long)before);
    usleep(4000000); // each client sends a request every 10 seconds
  }
  return 0;
}

void *receive_request(void *number) {
  long long start_time;
  time_t now;
  while (1) {
    start_time = bb_get(buffer);
    time(&now);
    long long elapsed = (long long)now - start_time;
    printf("\n[PROC T%i] Waiting time: %lld\n", *(int*)number, elapsed);
    usleep(50000); // the server spends 50 ms processing and responding to the request
  }
  return 0;
}


