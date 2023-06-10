#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>

int main() {

    int serverWriter = -1;
    int serverReader = -1;
    char readBuf[3] = { 0 };
    char betBuf[3] = { 0 };
    int bet = 0;
    int result;

    printf("Pipe-canele server opened\n");
    srand(time(NULL));

    if (mkfifo("sWriter", 0666) != 0 || mkfifo("cWriter", 0666) != 0) {
        fprintf(stderr, "Pipe creation failed\n");
        return 1;
    }
    
    serverReader = open("cWriter", O_RDONLY);
    serverWriter = open("sWriter", O_WRONLY);

    if (serverWriter < 0 || serverReader < 0) {
        fprintf(stderr, "Pipe acquisition failed\n");
        return 1;
    }

    while (1) {

        // result = 0;
        // if (!result){;}
        readBuf[0] = readBuf[1] = readBuf[2] = 0;
        betBuf[0] = betBuf[1] = betBuf[2] = 0;

        if (read(serverReader, readBuf, 3) < 0) {
            fprintf(stderr, "An error has occurred while reading from the client: %s\n", strerror(errno));
            break;
        }

        if (strncmp("DBL", readBuf, 3) != 0) {

            if (read(serverReader, &bet, sizeof(int)) < 0) {
                fprintf(stderr, "An error has occurred while reading from the client: %s\n", strerror(errno));
                break;
            }
            
            printf("The client has bet on %s for %d\n", readBuf, bet);

            for (int i = 0; i < 3; ++i) {
                betBuf[i] = 'A' + rand() % 4;
            }

            printf("Generated %s\n", betBuf);

            if (strncmp(readBuf, betBuf, 3) == 0) {

                if (betBuf[0] == betBuf[1] && betBuf[1] == betBuf[2]) {
                    result = bet * 3;
                } else if (betBuf[0] == betBuf[1] || 
                            betBuf[0] == betBuf[2] || 
                            betBuf[1] == betBuf[2]) {
                    result = bet * 2;
                }

            } else {
                
                result = 0;

            }

            if (write(serverWriter, betBuf, 3) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the client: %s\n", strerror(errno));
                break;
            }

            if (write(serverWriter, &result, sizeof(int)) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the client: %s\n", strerror(errno));
                break;
            }

        } else {

            printf("The client has doubled\n");

            for (int i = 0; i < 3; ++i) {
                betBuf[i] = 'A' + rand() % 4;
            }

            printf("Generated %s\n", betBuf);

            if (strncmp(readBuf, betBuf, 3) == 0) {

                result = bet * 2;

            } else {
                
                result = 0;

            }

            if (write(serverWriter, betBuf, 3) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the client: %s\n", strerror(errno));
                break;
            }

            if (write(serverWriter, &result, sizeof(int)) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the client: %s\n", strerror(errno));
                break;
            }

        }


        if (!bet) {
            printf("The client has decided to stop wasting money\n");
            break;
        }

    }

    close(serverWriter);
    close(serverReader);

    unlink("sWriter");
    unlink("cWriter");

    return 0;

}