#include "../include/clock.h"
#include <stdio.h>
#include <unistd.h>

void *clock_thread() {
    while (1) {
        usleep(100000);
        pthread_mutex_lock(&mutex);
        while (done < ntemps) {
            pthread_cond_wait(&cond, &mutex);
        }
        done = 0;
        pthread_cond_broadcast(&cond2);
        pthread_mutex_unlock(&mutex);
    }
}
