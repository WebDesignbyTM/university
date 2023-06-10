#define MAX 648
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {

    char lines[MAX];

    int fd = open("sample.txt", O_RDONLY);

    if (fd < 0) {
        perror("Cannot open file");
        exit(1);
    }

    int n = read(fd, lines, MAX);

    int no_of_lines = 0;
    for (int i = 0; i < n; ++i) {
        if (lines[i] == '\n') {
            ++no_of_lines;
        }
    }

    printf("Number of lines: %d\n", no_of_lines);

    return 0;
}