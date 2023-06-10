#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0
#define FAILURE -1

int insert(int *v1, int n1, int c1, int *v2, int n2, int pos) {

    if (c1 < n1 + n2)
        return FAILURE;

    for (int i = n1 + n2 - 1; i >= pos + n2; --i) {
        v1[i] = v1[i-n2];
    }

    for (int i = 0; i < n2; ++i)
        v1[i+pos] = v2[i];

    return SUCCESS;
}

int main() {

    int n1, n2, c1, pos;
    printf("Insert n1, n2, c1, pos: ");
    scanf("%d %d %d %d", &n1, &n2, &c1, &pos);

    int* v1 = (int*) calloc(c1, sizeof(int));
    int* v2 = (int*) calloc(n2, sizeof(int));

    printf("Insert the %d elements of the first array: ", n1);
    for (int i = 0; i < n1; ++i)
        scanf("%d", &v1[i]);

    printf("Insert the %d elements of the second array: ", n2);
    for (int i = 0; i < n2; ++i)
        scanf("%d", &v2[i]);

    insert(v1, n1, c1, v2, n2, pos);

    printf("The array is: ");
    for (int i = 0; i < n1 + n2; ++i)
        printf("%d ", v1[i]);

    free(v1);
    free(v2);

    return 0;
}