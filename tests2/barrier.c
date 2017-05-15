/*
  barrier_nocheck.c

  Sets up a barrier blocking n threads.
  If more than n arrive, the rest can go through.
  Doesn't check for errors.
 */

#include <pthread.h>
#include <stdio.h>

#include "barrier.h"

// Initialize the barrier to be size n
void init_barrier(struct Barrier* barP, int n) {
    pthread_mutex_init(&(barP->mutex), NULL);
    pthread_cond_init(&(barP->condvar), NULL);

    pthread_mutex_lock(&barP->mutex);
    barP->count = n;
    fprintf(stderr, "count initialized to: %d\n", barP->count);
    pthread_mutex_unlock(&barP->mutex);
}

// Wait at the barrier until all threads arrive
void wait_barrier(struct Barrier* barP) {
    pthread_mutex_lock(&barP->mutex); // Safely, access the barrier value
    --(barP->count);  // We're in!
    fprintf(stderr, "count decremented to: %d\n", barP->count);
    // Anyone left? Then release lock and wait.
    while (barP->count) 
      pthread_cond_wait(&barP->condvar, &barP->mutex); 
    pthread_cond_broadcast(&barP->condvar); // We're good!  Wake up!
    pthread_mutex_unlock(&barP->mutex); // And we're done with the lock.
} 

void release_barrier(struct Barrier* barP) {
    pthread_mutex_destroy(&(barP->mutex));
    pthread_cond_destroy(&(barP->condvar));
}

