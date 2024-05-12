#ifndef THEGHOSTCROSSWORD_THREADSUTILS_H
#define THEGHOSTCROSSWORD_THREADSUTILS_H

#include <pthread.h>
#include <stdbool.h>
#include "parameters.h"
#include "initBoard.h"

// Global variables
extern pthread_t threads[NUM_THREADS];
extern pthread_mutex_t lock;
extern pthread_cond_t workCond;
extern bool keepWorking;

void startThreadPool();

void stopThreadPool();


#endif //THEGHOSTCROSSWORD_THREADSUTILS_H
