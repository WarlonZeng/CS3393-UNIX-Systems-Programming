/*
  tbarriertest
  Based on Robbins & Robbins
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep

#include "barrier.h"  // initbarrier, waitbarrier

const int NUM_THREADS = 20;

// Thread function
void *printthread(void *arg);

int main(int argc, char *argv[]) {

    struct Barrier bar;
    init_barrier(&bar, NUM_THREADS);

    pthread_t tids[NUM_THREADS];

    // Set up the threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&tids[i], NULL, printthread, &bar))
            fprintf(stderr,"Error creating thread %d.\n", i);
    }
    
    // Wait till they're done.
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_join(tids[i], NULL))
            fprintf(stderr,"Error joining thread  %d.\n", i);
    }

    release_barrier(&bar);

    fprintf(stderr,"All threads complete.\n");
    return 0;
}
   
void *printthread(void *arg) {
    struct Barrier* barP = (struct Barrier*)arg;

    // Linux uses unsigned long for pthread_t
    fprintf(stderr,"This is the first print of thread %lu\n", (unsigned long)pthread_self());
    wait_barrier(barP);
    fprintf(stderr,"This is the second print of thread %lu\n", (unsigned long)pthread_self());
    return NULL;
}

