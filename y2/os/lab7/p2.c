#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int finished = 0;

void *threadFunc(void *pid) {

    int *id = (int*) pid;

    while (!finished) {
        printf("Thread %d is running.\n", *id);

    }

    printf("Thread %d is finished.\n", *id);

    return NULL;

}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return -1;
    }

    char command = 0;
    int n = atoi(argv[1]);
    pthread_t *threads = calloc(n, sizeof(pthread_t));
    int *ids = calloc(n, sizeof(int));

    for (int i = 0; i < n; ++i) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, threadFunc, (void*) &ids[i]);
    }

    while (!finished) {
        finished = (command == 'q');
        command = fgetc(stdin);
    }

    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("The main thread is finished.\n");

    free(threads);
    free(ids);

    return 0;
}