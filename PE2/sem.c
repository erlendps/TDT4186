#include "sem.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

/* Opaque type of a semaphore. 
 * ...you need to figure out the contents of struct SEM yourself!
 */
typedef struct SEM {
  int value;
  pthread_cond_t cond;
  pthread_mutex_t mutex;
} SEM;

/* Creates a new semaphore.
 *
 * This function creates a new semaphore. If an error occurs during the 
 * initialization, the implementation shall free all resources already 
 * allocated so far.
 *
 * Parameters:
 *
 * initVal      the initial value of the semaphore
 *
 * Returns:
 *
 * handle for the created semaphore, or NULL if an error occured.
 */

SEM *sem_init(int initVal) {
  SEM* sem = malloc(sizeof(struct SEM));//malloc(sizeof(*sem)); // TODO: how much is needed?
  sem->value = initVal;
  if (pthread_cond_init(&sem->cond, NULL) > 0) {
    sem_del(sem);
    return NULL;
  }
  if (pthread_mutex_init(&sem->mutex, NULL) > 0) {
    sem_del(sem);
    return NULL;
  }
  return sem;
}

/* Destroys a semaphore and frees all associated resources.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to destroy
 *
 * Returns:
 *
 * 0 on success, negative value on error. 
 *
 * In case of an error, not all resources may have been freed, but 
 * nevertheless the semaphore handle must not be used any more.
 */

int sem_del(SEM *sem) {
  int result = 0;
  if (pthread_mutex_destroy(&sem->mutex) > 0) {
    result -= 1;
  }
  if (pthread_cond_destroy(&sem->cond) > 0) {
    result -= 2;
  }
  free(sem);
  return result;
}

/* P (wait) operation.
 * 
 * Attempts to decrement the semaphore value by 1. If the semaphore value 
 * is 0, the operation blocks until a V operation increments the value and 
 * the P operation succeeds.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to decrement
 */

void P(SEM *sem) {
  pthread_mutex_lock(&sem->mutex);
  while (sem->value <= 0) {
    pthread_cond_wait(&sem->cond, &sem->mutex);
  }
  sem->value--;
  pthread_mutex_unlock(&sem->mutex);
}

/* V (signal) operation.
 *
 * Increments the semaphore value by 1 and notifies P operations that are 
 * blocked on the semaphore of the change.
 *
 * Parameters:
 *
 * sem           handle of the semaphore to increment
 */

void V(SEM *sem) {
  pthread_mutex_lock(&sem->mutex);
  sem->value++;
  pthread_cond_signal(&sem->cond);
  pthread_mutex_unlock(&sem->mutex);
}

int get_value(SEM *sem) {
  return sem->value;
}
