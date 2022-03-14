#include "bbuffer.h"
#include "sem.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

/*
 * Bounded Buffer implementation to manage int values that supports multiple 
 * readers and writers.
 *
 * The bbuffer module uses the sem module API to synchronize concurrent access 
 * of readers and writers to the bounded buffer.
 */

/* Opaque type of a Bounded Buffer.
 * ...you need to figure out the contents of struct BNDBUF yourself
 */

typedef struct BNDBUF {
  uint8_t* buffer;
  size_t head;
  size_t tail;
  size_t maxsize;
  SEM *full_sem;
  SEM *empty_sem;
} BNDBUF;

/* Checks if the given buffer is empty.
 * 
 * Returns 0 if its not empty, 1 if its empty.
 */
bool bb_is_empty(BNDBUF *buffer) {
  return (buffer->head == buffer->tail);
}

/* Checks if the given buffer is full.
 * 
 * Returns 0 if its not full, 1 if its full.
 */
bool bb_is_full(BNDBUF *buffer) {
  return (buffer->head == buffer->tail + 1);
}


/* Creates a new Bounded Buffer. 
 *
 * This function creates a new bounded buffer and all the helper data 
 * structures required by the buffer, including semaphores for 
 * synchronization. If an error occurs during the initialization the 
 * implementation shall free all resources already allocated by then.
 *
 * Parameters:
 *
 * size     The number of integers that can be stored in the bounded buffer.
 *
 * Returns:
 *
 * handle for the created bounded buffer, or NULL if an error occured.
 */

BNDBUF *bb_init(unsigned int size) {
  BNDBUF *bbuffer = malloc(sizeof(struct BNDBUF));
  bbuffer->full_sem = sem_init(size);
  bbuffer->empty_sem = sem_init(0);
  bbuffer->head = 0;
  bbuffer->tail = 0;
  bbuffer->maxsize = size;
  bbuffer->buffer = malloc(size*sizeof(uint8_t));
  return bbuffer;
}

/* Destroys a Bounded Buffer. 
 *
 * All resources associated with the bounded buffer are released.
 *
 * Parameters:
 *
 * bb       Handle of the bounded buffer that shall be freed.
 */

void bb_del(BNDBUF *bb) {
  free(bb);
}

/* Retrieve an element from the bounded buffer.
 *
 * This function removes an element from the bounded buffer. If the bounded 
 * buffer is empty, the function blocks until an element is added to the 
 * buffer.
 *
 * Parameters:
 *
 * bb         Handle of the bounded buffer.
 *
 * Returns:
 *
 * the int element
 */

int bb_get(BNDBUF *bb) {
  P(bb->empty_sem);
  V(bb->full_sem);
  uint8_t element = *(bb->buffer + bb->tail);
  (bb->tail) = bb->tail + 1 % bb->maxsize;
  return element;
}

/* Add an element to the bounded buffer. 
 *
 * This function adds an element to the bounded buffer. If the bounded 
 * buffer is full, the function blocks until an element is removed from 
 * the buffer.
 *
 * Parameters:
 *
 * bb     Handle of the bounded buffer.
 * fd     Value that shall be added to the buffer.
 *
 * Returns:  bbuffer->buffer = 

 *
 * the int element
 */

void bb_add(BNDBUF *bb, int fd) {
  P(bb->full_sem);
  V(bb->empty_sem);
  bb->buffer[bb->head] = fd;
  bb->head = bb->head + 1 % bb->maxsize;
}

int main(void) {
  BNDBUF *buffer = bb_init(4);
  bb_add(buffer, 32);
  bb_add(buffer, 39);
  bb_add(buffer, 12);
  bb_add(buffer, 120);
  uint8_t x;
  while (!bb_is_empty(buffer)) {
    x = bb_get(buffer);
    printf("%i\n", x);
    
  }
  return 0;
}
