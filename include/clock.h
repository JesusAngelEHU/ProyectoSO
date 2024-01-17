#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>

extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern pthread_cond_t cond2;
extern int clk;
extern int done;
extern int ntemps;

void *clock_thread();

#endif
