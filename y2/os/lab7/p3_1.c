#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define LL long long

typedef struct factLimits {
    LL lo, hi;
} factLimits;

void *calculateProduct(void* vpLimits) {

    factLimits *pLimits = (factLimits*) vpLimits;
    LL *res = malloc(sizeof(LL));
    *res = 1;

    for (LL i = pLimits->lo; i <= pLimits->hi; ++i) 
        *res *= i;

    return (void*)res;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <number_of_threads> <factorial_limit>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    LL m = atoll(argv[2]);
    LL aux = 1;
    LL sol = 1;
    pthread_t *threads = calloc(n, sizeof(pthread_t));
    factLimits *computationLimits = calloc(n, sizeof(factLimits));
    struct timeval tvs, tvf;
    struct timezone tz;

    gettimeofday(&tvs, &tz);

    // Fiecare thread va calcula produsul a maxim m / n numere consecutive
    // din intervalul [1, m]
    // In cazul in care sunt mai multe threaduri decat intervale de calcul
    // ultimele threaduri nu vor calcula nimic
    for (int i = 0; i < n; ++i) {

        // Tratez cazul in care threadul curent este redundant
        if (aux > m) {
            computationLimits[i].lo = 1;
            computationLimits[i].hi = -1;
        } else {
            computationLimits[i].lo = aux;
            aux += (m / n - 1 > 0) ? m / n - 1 : 0;
            // Garantez incadrarea intervalului de lucru in [1, m]
            computationLimits[i].hi = (aux > m) ? m : aux;
            ++aux;
        }

        pthread_create(&threads[i], NULL, calculateProduct, (void*) &computationLimits[i]);

    }

    for (int i = 0; i < n; ++i) {
        void *retVal = NULL;
        pthread_join(threads[i], &retVal);

        printf("Thread %d returned %lld for [%lld, %lld]\n", i, *(LL*)retVal, 
                computationLimits[i].lo, computationLimits[i].hi);

        sol *= *(LL*)retVal;

        free(retVal);
    }

    gettimeofday(&tvf, &tz);

    printf("The program finished in %ld seconds and %ld miliseconds.\n", 
            tvf.tv_sec - tvs.tv_sec, tvf.tv_usec - tvs.tv_usec);
    printf("The program calculated %lld\n", sol);

    free(threads);
    free(computationLimits);

    return 0;
}