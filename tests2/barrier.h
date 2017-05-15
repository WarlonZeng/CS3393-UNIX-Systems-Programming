struct Barrier {
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t condvar;
};

void init_barrier(struct Barrier* barP, int n);
void wait_barrier(struct Barrier* barP);
void release_barrier(struct Barrier* barP);
