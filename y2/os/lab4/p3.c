#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define BUFFER_SIZE 2048
// poti testa valori diferite ale dimensiunii offsetului
// cat timp buffer-ul e mai mare decat string-ul introdus,
// ar trebui sa mearga

int main(int argc, char **argv) {
    int fd, strOffset, fPos;
    int bufIdx = 0;
    int readBytes[2] = { 0 };
    char buf[2][BUFFER_SIZE] = { 0 };
    int retVal = 0;
    off_t fOffset = 0;

    if (argc < 3) {
        printf("Usage: %s <file> <offset> <string>", argv[0]);
        return 1;
    }

    if ((fd = open(argv[1], O_RDWR)) == -1) {
        perror("Failed to open the specified file\n");
        return 1;
    }

    fPos = atoi(argv[2]);

    do {

        if (lseek(fd, fPos, SEEK_SET) == -1) {
            perror("Failed to set file pointer\n");
            retVal = 1;
            break;
        }

        strOffset = strlen(argv[3]);

        if ((readBytes[bufIdx] = read(fd, buf[bufIdx], BUFFER_SIZE)) == -1) {
            perror("Failed to read from file\n");
            retVal = 1;
            break;
        }

        // citeste alternativ in fiecare buffer si apoi muta continutul primului buffer
        // cu offsetul generat de string-ul de input
        while ((readBytes[bufIdx^1] = read(fd, buf[bufIdx^1], BUFFER_SIZE)) > 0) {
            fOffset = lseek(fd, 0, SEEK_CUR);
            lseek(fd, fOffset + strOffset - readBytes[bufIdx^1] - readBytes[bufIdx], SEEK_SET);
            write(fd, buf[bufIdx], readBytes[bufIdx]);
            lseek(fd, fOffset, SEEK_SET);

            memset(buf[bufIdx], 0, BUFFER_SIZE);
            bufIdx ^= 1;
        }

        lseek(fd, strOffset - readBytes[bufIdx^1] - readBytes[bufIdx], SEEK_CUR);
        write(fd, buf[bufIdx], readBytes[bufIdx]);

    } while (0);

    lseek(fd, fPos, SEEK_SET);
    write(fd, argv[3], strOffset);

    close(fd);

    return retVal;
}