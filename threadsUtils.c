#include "threadsUtils.h"
#include "parameters.h"

void startThreadPool() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&workCond, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, workerFunction, NULL);
    }
}

void stopThreadPool() {
    pthread_mutex_lock(&lock);
    keepWorking = false;
    pthread_cond_broadcast(&workCond);  // Wake up all threads to let them exit
    pthread_mutex_unlock(&lock);

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&workCond);
}