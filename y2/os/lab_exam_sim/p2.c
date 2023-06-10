#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

typedef struct __THREAD_ARGS {
    pthread_mutex_t* mutex;
    pthread_cond_t* entryCond;
    pthread_cond_t* exitCond;
    int runningThreads;
} THREAD_ARGS, *PTHREAD_ARGS;

void* subsidiaryCompany(void* args) {
    
    PTHREAD_ARGS params = (PTHREAD_ARGS) args;

    pthread_mutex_lock(params->mutex);
    params->runningThreads += 1;
    while (params->runningThreads != 3) 
        pthread_cond_wait(params->entryCond, params->mutex);
    pthread_mutex_unlock(params->mutex);

    printf("Subsidiary company launched\n");

    srand(0);
    sleep(rand() % 4);

    printf("Subsidiery company product exited the market\n");

    pthread_mutex_lock(params->mutex);
    params->runningThreads -= 1;
    pthread_mutex_unlock(params->mutex);

    pthread_cond_signal(params->exitCond);

    return NULL;
}

void* proprietaryCompany(void* args) {

    PTHREAD_ARGS params = (PTHREAD_ARGS) args;

    pthread_mutex_lock(params->mutex);
    params->runningThreads += 1;
    while (params->runningThreads != 3) 
        pthread_cond_wait(params->entryCond, params->mutex);
    pthread_mutex_unlock(params->mutex);

    pthread_cond_broadcast(params->entryCond);

    printf("Proprietary company launched\n");

    sleep(2);

    pthread_mutex_lock(params->mutex);
    while (params->runningThreads > 1) {
        pthread_cond_wait(params->exitCond, params->mutex);
    }
    params->runningThreads -= 1;
    pthread_mutex_unlock(params->mutex);

    printf("Proprietary company product exited the market\n");

    return NULL;
}

int main(int argc, char** argv) {

    int cycles = 0;
    char *endPtr = NULL;
    pthread_mutex_t mutex;
    pthread_cond_t entryCond;
    pthread_cond_t exitCond;
    pthread_t threads[3];
    void* results[3];
    THREAD_ARGS args = {
        &mutex,
        &entryCond,
        &exitCond,
        0
    };

    if (argc != 2) {
        printf("Usage: %s <cycles>\n", argv[0]);
        return 1;
    }

    cycles = strtol(argv[1], &endPtr, 10);
    if (cycles == 0 && endPtr == argv[1]) {
        printf("Please provide a number\n");
        return 1;
    }

    for (int i = 0; i < cycles; ++i) {
        
        printf("Cycle %d:\n", i + 1);

        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&entryCond, NULL);
        pthread_cond_init(&exitCond, NULL);

        pthread_create(&threads[0], NULL, subsidiaryCompany, &args);
        pthread_create(&threads[1], NULL, subsidiaryCompany, &args);
        sleep(2);
        pthread_create(&threads[2], NULL, proprietaryCompany, &args);

        for (int i = 0; i < 3; ++i) {
            pthread_join(threads[i], &results[i]);
        }

        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&entryCond);
        pthread_cond_destroy(&exitCond);
    }

    return 0;
}