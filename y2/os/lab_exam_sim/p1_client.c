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

    int clientWriter = -1;
    int clientReader = -1;
    char writeBuf[3];
    int bet = 0;
    int cannotDbl = 0;

    printf("Pipe-canele client opened\n");
    
    clientWriter = open("cWriter", O_WRONLY);
    clientReader = open("sWriter", O_RDONLY);

    if (clientWriter < 0 || clientReader < 0) {
        fprintf(stderr, "Pipe acquisition failed\n");
        return 1;
    }

    while (1) {
        
    loopStart:

        printf("Please input a 3-character string [A-D] and an "
                "amount (\"DBL\" to double the previous bet):\n");
        scanf("%s", writeBuf);

        for (int i = 0; i < 3; ++i) {
            if (writeBuf[i] < 'A' || 'D' < writeBuf[i]) {
                fprintf(stderr, "Please input a valid string\n");
                goto loopStart;
            }
        } 

        if (strncmp("DBL", writeBuf, 3) == 0) {

            if (cannotDbl) {
                printf("Bruh\n");
                goto loopStart;
            }

            cannotDbl = 1;

            if (write(clientWriter, writeBuf, 3) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the server: %s\n", strerror(errno));
                return 1;
            }

            if (write(clientWriter, &bet, sizeof(int)) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the server: %s\n", strerror(errno));
                return 1;
            }

        } else {

            scanf("%d", &bet);

            if (write(clientWriter, writeBuf, 3) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the server: %s\n", strerror(errno));
                return 1;
            }

            if (write(clientWriter, &bet, sizeof(int)) < 0) {
                fprintf(stderr, "An error has occurred whie writing to the server: %s\n", strerror(errno));
                return 1;
            }

            if (bet == 0) {
                printf("Good choice\n");
                break;
            }

        }

        if (read(clientReader, writeBuf, 3) < 0) {
            fprintf(stderr, "An error has occurred whie reading from the server: %s\n", strerror(errno));
            return 1;
        }

        if (read(clientReader, &bet, sizeof(int)) < 0) {
            fprintf(stderr, "An error has occurred whie reading from the server: %s\n", strerror(errno));
            return 1;
        }

        printf("Received %s %d\n", writeBuf, bet);

    }
    
    close(clientWriter);
    close(clientReader);

    return 0;

}