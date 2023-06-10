#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define CONNECTION_SLEEP 3
#define CLIENT_VAR 3
#define CLIENT_MIN 4
#define MAXIMUM_CONNECTIONS 1000

int finished = 0;

void *controlFunction() {

    char command = 0;

    while (!finished) {
        command = fgetc(stdin);
        finished = (command == 'x');
    }

    printf("The control thread is finished.\n");

    return 0;

}

void *clientThread() {

    pthread_t selfId = pthread_self();
    printf("%ld Starting...\n", selfId);
    sleep(rand() % CLIENT_VAR + CLIENT_MIN);
    printf("%ld Ending...\n", selfId);
    return 0;

}

void *connectionFunction() {

    int nextThrd = 0;
    pthread_t threads[MAXIMUM_CONNECTIONS];

    while (!finished) {
        pthread_create(&threads[nextThrd], NULL, clientThread, NULL);
        sleep(CONNECTION_SLEEP);
        ++nextThrd;
    }

    for (int i = 0; i < nextThrd; ++i)
        pthread_join(threads[i], NULL);

    printf("The connection thread is finished.\n");

    return 0;

}


int main() {
    
    pthread_t ctrlThread;
    pthread_t connectionThread;
    
    pthread_create(&ctrlThread, NULL, controlFunction, NULL);
    pthread_create(&connectionThread, NULL, connectionFunction, NULL);

    pthread_join(ctrlThread, NULL);
    pthread_join(connectionThread, NULL);

    return 0;
}