#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**  argv) {

    if (argc < 2) {
        fprintf((FILE*) STDERR_FILENO, "Usage: %s <filename>", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf((FILE*) STDERR_FILENO, "Failed to open %s", argv[1]);
        return 2;
    }

    off_t begining = lseek(fd, 0, SEEK_SET);
    off_t end = lseek(fd, 0, SEEK_END);
    printf("The total number of bytes is %ld\n", end - begining);

    close(fd);

    return 0;
}