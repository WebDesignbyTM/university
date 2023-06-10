#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define LL long long

typedef struct factLimits {
    LL lo, hi;
} factLimits;

void *getPrimes(void* vpLimits) {

    factLimits *pLimits = (factLimits*) vpLimits;
    int ok = 1;

    for (LL i = pLimits->lo; i <= pLimits->hi; ++i) {
        ok = 1;

        if (!i || i == 1 || i == 2)
            continue;

        for (LL j = 2; j * j <= i && ok; ++j)
            ok = (i % j != 0);

        if (ok)
            printf("%lld ", i);
    }

    return NULL;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <number_of_threads> <factorial_limit>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    LL m = atoll(argv[2]);
    LL aux = 1;
    pthread_t *threads = calloc(n, sizeof(pthread_t));
    factLimits *computationLimits = calloc(n, sizeof(factLimits));
    struct timeval tvs, tvf;
    struct timezone tz;

    gettimeofday(&tvs, &tz);

    // Fiecare thread va verifica primalitatea a maxim m / n numere consecutive
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

        pthread_create(&threads[i], NULL, getPrimes, (void*) &computationLimits[i]);

    }

    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&tvf, &tz);

    printf("\nThe program finished in %ld seconds and %ld miliseconds.\n", 
            tvf.tv_sec - tvs.tv_sec, tvf.tv_usec - tvs.tv_usec);

    free(threads);
    free(computationLimits);

    return 0;
}