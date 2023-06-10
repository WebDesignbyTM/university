#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#define byte unsigned char
#define dword long unsigned int

int processNumber(dword* pNum) {

    if (pNum == NULL) {
        fprintf(stderr, "Error processing number address.\n");
        return 1;
    }

    byte* pByte = (byte*) pNum;
    byte aux;

    aux = *pByte;

    *pByte = *(pByte + 3);
    *(pByte + 3) = aux;

    aux = *(pByte + 1);

    *(pByte + 1) = *(pByte + 2);
    *(pByte + 2) = aux;

    return 0;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    if (isalpha(argv[1][0])) {
        fprintf(stderr, "Please be advised that the number you entered may not be valid\n");
    }

    dword number = strtoul(argv[1], NULL, 10);

    printf("Initial number: %lu\n", number);
    processNumber(&number);
    printf("Processed number: %lu\n", number);

    return 0;
}