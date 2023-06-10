#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {

    int fd = -1;
    int offset = 0;
    int blocks = 0;
    struct stat fdata;

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    if ((fd = open(argv[1], 0)) == -1) {
        printf("Error opening %s (%s)\n", argv[1], strerror(errno));
        return 1;
    }

    fstat(fd, &fdata);
    offset = lseek(fd, 0, SEEK_END);
    blocks = offset / fdata.st_blksize + (offset % fdata.st_blksize != 0);

    printf("The file appears to have %d bytes, and %d blocks\n", offset, blocks);
    printf("Stat reports %ld bytes and %ld blocks\n", fdata.st_size, fdata.st_blocks);

    close(fd);

    return 0;
}