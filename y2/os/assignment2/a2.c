#include "a2_helper.h"
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define PROC_CREAT_ERR      -1
#define SEM_ERR             -2
#define STATUS_ERR          -3
#define THREAD_CREAT_ERR    -4
#define PROC3_THREADS       48
#define PROC5_THREADS       5
#define PROC8_THREADS       5
#define PROC_MESSAGE        "An error occurred during process creation\n"
#define THREAD_MESSAGE      "An error occurred during thread creation\n"
#define SEMAPHORE_MESSAGE   "An error occurred during semaphore creation\n"
#define STATUS_MESSAGE      "An error occurred during thread execution\n"
#define ERROR(x)            printf(x);

// pornesc de la premiza ca un esec in stergerea unui semafor nu e 
// o eroare ce trebuie tratata (nu pare a fi un mem leak);
// de asemenea, a trata fiecare eroare din functiile pt threaduri imi 
// pare a fi exagerat; chiar nu stiu daca ar trebui sau nu facut

typedef struct _THREAD_PARAMS {
    void* extra;
    int procId, threadId;
} *PTHREAD_PARAMS, THREAD_PARAMS;

typedef struct _PROC3_PARAMS {
    sem_t* barrier;
    sem_t* entryGuard;
    sem_t* exitGuard;
    pthread_mutex_t* mutex;
    pthread_cond_t* condVar;
    int runningThreads;
} *PPROC3_PARAMS, PROC3_PARAMS;

void* process8Thread2(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    sem_t* t54s = sem_open("t54s", 0);

    info(BEGIN, tParams.procId, tParams.threadId);

    info(END, tParams.procId, tParams.threadId);
    sem_post(t54s);
    return 0;
}

void* process8Thread4(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    sem_t* t54e = sem_open("t54e", 0);

    sem_wait(t54e);
    info(BEGIN, tParams.procId, tParams.threadId);

    info(END, tParams.procId, tParams.threadId);
    return 0;
}

void* genericThread(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    info(BEGIN, tParams.procId, tParams.threadId);

    info(END, tParams.procId, tParams.threadId);
    return 0;
}

void* process3Thread(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    PPROC3_PARAMS p3Params = (PPROC3_PARAMS) tParams.extra;

    sem_wait(p3Params->entryGuard);
    sem_post(p3Params->entryGuard);

    sem_wait(p3Params->barrier);
    info(BEGIN, tParams.procId, tParams.threadId);

    pthread_mutex_lock(p3Params->mutex);
    ++(p3Params->runningThreads);
    if (p3Params->runningThreads == 4)
        pthread_cond_signal(p3Params->condVar);
    pthread_mutex_unlock(p3Params->mutex);

    sem_wait(p3Params->exitGuard);
    sem_post(p3Params->exitGuard);

    info(END, tParams.procId, tParams.threadId);
    sem_post(p3Params->barrier);

    return 0;
}

void* process3Thread12(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    PPROC3_PARAMS p3Params = (PPROC3_PARAMS) tParams.extra;

    sem_wait(p3Params->barrier);
    info(BEGIN, tParams.procId, tParams.threadId);

    sem_post(p3Params->entryGuard);

    pthread_mutex_lock(p3Params->mutex);
    ++(p3Params->runningThreads);
    while (p3Params->runningThreads != 4)
        pthread_cond_wait(p3Params->condVar, p3Params->mutex);
    pthread_mutex_unlock(p3Params->mutex);

    info(END, tParams.procId, tParams.threadId);
    sem_post(p3Params->exitGuard);
    sem_post(p3Params->barrier);

    return 0;
}

void process8() {
    pthread_t tid[PROC8_THREADS];
    void* twstatus[PROC8_THREADS];
    THREAD_PARAMS tParams[PROC8_THREADS];
    int retVal = 0;
    void* (*functions[PROC8_THREADS])(void*) = {
        genericThread,
        process8Thread2,
        genericThread,
        process8Thread4,
        genericThread,
    };
    info(BEGIN, 8, 0);

    for (int i = 0; i < PROC8_THREADS; ++i) {
        tParams[i].extra = NULL;
        tParams[i].procId = 8;
        tParams[i].threadId = i + 1;
    }

    for (int i = 0; i < PROC8_THREADS; ++i)
        if (pthread_create(&tid[i], NULL, functions[i], &tParams[i]) != 0) {
            ERROR(THREAD_MESSAGE);
            retVal = THREAD_CREAT_ERR;
        }

    for (int i = 0; i < PROC8_THREADS; ++i) {
        pthread_join(tid[i], &twstatus[i]);
        if (twstatus[i] != 0) {
            ERROR(STATUS_MESSAGE);
            retVal = STATUS_ERR;
        }
    }

    info(END, 8, 0);
    exit(retVal);
}

void process3() {
    int retVal = 0;
    pid_t pid = -1;
    int wstatus = -1;
    pthread_t tid[PROC3_THREADS];
    void* twstatus[PROC3_THREADS];
    THREAD_PARAMS tParams[PROC3_THREADS];
    sem_t bottleNeck;
    sem_t exitGuard;
    sem_t entryGuard;
    pthread_mutex_t mutex;
    pthread_cond_t condVar;
    PROC3_PARAMS p3Params = {
        &bottleNeck,
        &entryGuard,
        &exitGuard,
        &mutex,
        &condVar,
        0
    };
    info(BEGIN, 3, 0);

    do {
        pid = fork();
        if (pid == 0) {
            process8();
        } else if (pid < 0) {
            ERROR(PROC_MESSAGE);
            retVal = PROC_CREAT_ERR;
            break;
        }


        if (sem_init(&bottleNeck, 0, 4) == -1) {
            ERROR(SEMAPHORE_MESSAGE);
            retVal = SEM_ERR;
            break;
        }
        if (sem_init(&entryGuard, 0, 0) == -1) {
            ERROR(SEMAPHORE_MESSAGE);
            retVal = SEM_ERR;
            break;
        }
        if (sem_init(&exitGuard, 0, 0) == -1) {
            ERROR(SEMAPHORE_MESSAGE);
            retVal = SEM_ERR;
            break;
        }
        if (pthread_mutex_init(&mutex, NULL) != 0) {
            ERROR(SEMAPHORE_MESSAGE);
            retVal = SEM_ERR;
            break;
        }
        if (pthread_cond_init(&condVar, NULL) != 0) {
            ERROR(SEMAPHORE_MESSAGE);
            retVal = SEM_ERR;
            break;
        }

        for (int i = 0; i < PROC3_THREADS; ++i) {
            tParams[i].extra = (void*) &p3Params;
            tParams[i].procId = 3;
            tParams[i].threadId = i + 1;
        }

        for (int i = 0; i < PROC3_THREADS; ++i)
            if (i != 11) {
                if (pthread_create(&tid[i], NULL, process3Thread, &tParams[i]) != 0) {
                    ERROR(THREAD_MESSAGE);
                    retVal = THREAD_CREAT_ERR;
                }
            } else {
                if (pthread_create(&tid[i], NULL, process3Thread12, &tParams[i]) != 0) {
                    ERROR(THREAD_MESSAGE);
                    retVal = THREAD_CREAT_ERR;
                }
            }

        for (int i = 0; i < PROC3_THREADS; ++i) {
            pthread_join(tid[i], &twstatus[i]);
            if (twstatus[i] != 0) {
                ERROR(STATUS_MESSAGE);
                retVal = STATUS_ERR;
            }
        }

        sem_destroy(&bottleNeck);
        sem_destroy(&exitGuard);
        sem_destroy(&entryGuard);
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&condVar);

        if (pid != -1) {
            waitpid(pid, &wstatus, 0);
            if (wstatus != 0) {
                ERROR(STATUS_MESSAGE);
                retVal = STATUS_ERR;
            }
        }
    } while(0);

    info(END, 3, 0);
    exit(retVal);
}

void genericChildProcess(int procId, int depth) {
    int retVal = 0;
    pid_t pid = -1;
    int wstatus = -1;
    info(BEGIN, procId, 0);

    if (depth != 0) {
        pid = fork();
        if (pid == 0) {
            // da, am gasit o functie matematica pt care f(4) = 6 si f(6) = 7
            // ce o sa faci in legatura cu asta?
            genericChildProcess(procId + 1 + procId % 3, depth - 1);
        } else if (pid < 0) {
            ERROR(PROC_MESSAGE);
            retVal = PROC_CREAT_ERR;
        }
    }

    if (pid != -1) {
        waitpid(pid, &wstatus, 0);
        if (wstatus != 0) {
            ERROR(STATUS_MESSAGE);
            retVal = STATUS_ERR;
        }
    }

    info(END, procId, 0);
    exit(retVal);
}

void process2() {
    int retVal = 0;
    pid_t pid = -1;
    int wstatus = -1;
    info(BEGIN, 2, 0);

    pid = fork();
    if (pid == 0) {
        process3();
    } else if (pid < 0) {
        ERROR(PROC_MESSAGE);
        retVal = PROC_CREAT_ERR;
    }

    if (pid != -1) {
        waitpid(pid, &wstatus, 0);
        if (wstatus != 0) {
            ERROR(STATUS_MESSAGE);
            retVal = STATUS_ERR;
        }
    }

    info(END, 2, 0);
    exit(retVal);
}

void* process5Thread1(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    sem_t *semaphores = tParams.extra;
    info(BEGIN, tParams.procId, tParams.threadId);

    sem_post(&semaphores[1]);
    sem_wait(&semaphores[0]);

    info(END, tParams.procId, tParams.threadId);
    return 0;
}

void* process5Thread2(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    sem_t *semaphores = tParams.extra;

    sem_wait(&semaphores[1]);
    info(BEGIN, tParams.procId, tParams.threadId);

    info(END, tParams.procId, tParams.threadId);
    sem_post(&semaphores[0]);

    return 0;
}

void* process5Thread4(void* args) {
    THREAD_PARAMS tParams = *(PTHREAD_PARAMS) args;
    sem_t* t54s = sem_open("t54s", 0);
    sem_t* t54e = sem_open("t54e", 0);

    sem_wait(t54s);
    info(BEGIN, tParams.procId, tParams.threadId);

    info(END, tParams.procId, tParams.threadId);
    sem_post(t54e);

    return 0;
}

void process5() {
    pthread_t tid[PROC5_THREADS];
    sem_t internalSync[2];
    THREAD_PARAMS tParams[PROC5_THREADS] = {{ NULL, 5, 0 }};
    void* (*functions[PROC5_THREADS])(void*) = {
        process5Thread1,
        process5Thread2,
        genericThread,
        process5Thread4,
        genericThread
    };
    info(BEGIN, 5, 0);

    sem_init(&internalSync[0], 0, 0);
    sem_init(&internalSync[1], 0, 0);
    tParams[0].extra = (void*) internalSync;
    tParams[1].extra = (void*) internalSync;
    for (int i = 0; i < PROC5_THREADS; ++i) {
        tParams[i].procId = 5;
        tParams[i].threadId = i + 1;
    }

    for (int i = 0; i < PROC5_THREADS; ++i)
        pthread_create(&tid[i], NULL, functions[i], &tParams[i]);

    for (int i = 0; i < PROC5_THREADS; ++i)
        pthread_join(tid[i], NULL);

    sem_destroy(&internalSync[0]);
    sem_destroy(&internalSync[1]);

    info(END, 5, 0);
    exit(0);
}

int main(){
    init();
    int retVal = 0;
    pid_t children[3] = { -1 };
    int wstatus[3] = { -1 };
    sem_t* t54s;
    sem_t* t54e;
    info(BEGIN, 1, 0);

    do {
        if ((t54s = sem_open("t54s", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0)) == SEM_FAILED) {
            retVal = SEM_ERR;
            break;
        }
        if ((t54e = sem_open("t54e", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0)) == SEM_FAILED) {
            retVal = SEM_ERR;
            break;
        }
        

        children[0] = fork();
        if (children[0] == 0) {
            process2();
        } else if (children[0] < 0) {
            ERROR(PROC_MESSAGE);
            retVal = PROC_CREAT_ERR;
            break;
        }

        children[1] = fork();
        if (children[1] == 0) {
            genericChildProcess(4, 2);
        } else if (children[0] < 0) {
            ERROR(PROC_MESSAGE);
            retVal = PROC_CREAT_ERR;
            break;
        }

        children[2] = fork();
        if (children[2] == 0) {
            process5();
        } else if (children[2] < 0) {
            ERROR(PROC_MESSAGE);
            retVal = PROC_CREAT_ERR;
            break;
        }

        for (int i = 0; i < 3; ++i) {
            waitpid(children[i], &wstatus[i], 0);
            if (wstatus[i] != 0)
                retVal = STATUS_ERR;
        }
    } while (0);
    
    if (t54s != SEM_FAILED)
        sem_unlink("t54s");
    if (t54e != SEM_FAILED)
        sem_unlink("t54e");

    info(END, 1, 0);
    return retVal;
}