#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define SUCCESS 0
#define FAILURE -1
#define BUFFER_SIZE 2048
#define LINE_LENGTH 100

/**
 * @brief Get the line object
 * 
 * @param fd [in] File descriptor
 * @param line [in] Pointer to buffer
 * @param line_no [in] Line number (0-indexed)
 * @param max_length [in] Buffer length
 * @param line_length [out] Pointer to number of bytes read
 * @return int 0 for success, -1 otherwise
 */
int get_line(int fd, char *line, int line_no, int max_length, int *line_length) {
    char buf[BUFFER_SIZE];
    int read_bytes = 0;
    int idx = 0;

    if (line == NULL) {
        perror("Invalid line pointer\n");
        return FAILURE;
    }

    if (line_length == NULL) {
        perror("Invalid line length pointer\n");
        return FAILURE;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Failed to position file offset\n");
        return FAILURE;
    }

    memset(line, 0, max_length);
    *line_length = 0;

    while (line_no > -1 && (read_bytes = read(fd, buf, BUFFER_SIZE)) > 0) {

        for (idx = 0; idx < read_bytes && line_no > -1; ++idx) {
            if (buf[idx] == '\n') {
                --line_no;
                continue;
            }

            if (!line_no) {
                if (*line_length == max_length)
                    return FAILURE;
                line[(*line_length)++] = buf[idx];
            }
        }
    }

    if (line_no != -1 && (lseek(fd, 0, SEEK_CUR) != lseek(fd, 0, SEEK_END) && line_no))
        return FAILURE;

    return SUCCESS;
}

/**
 * @brief Write from one file to another, reversing its lines
 * 
 * @param fd Source file descriptor
 * @param hd Destination file descriptor
 * @return int 0 for success, -1 otherwise
 */
int reverse_lines(int fd, int hd) {
    char buf[BUFFER_SIZE];
    char line[LINE_LENGTH];
    int read_bytes;
    int total_lines = 1;

    if (lseek(fd, 0, SEEK_SET) == -1 || lseek(hd, 0, SEEK_SET) == -1) {
        perror("Failed to position file offset\n");
        return FAILURE;
    }

    while ((read_bytes = read(fd, buf, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < read_bytes; ++i)
            total_lines += (buf[i] == '\n');
    }

    for (int i = total_lines - 1; i >= 0; --i) {
        if (get_line(fd, line, i, LINE_LENGTH, &read_bytes) == FAILURE) {
            perror("Failed to get line\n");
            return FAILURE;
        }

        if (write(hd, line, read_bytes) == -1) {
            perror("Failed to write to file\n");
            return FAILURE;
        }

        if (i) {
            if (write(hd, "\n", sizeof(char)) == -1) {
                perror("Failed to write to file\n");
                return FAILURE;
            }
        }
    }

    return SUCCESS;
}

int main(int argc, char **argv) {
    int fd = -1;
    int hd = -1;
    int read_bytes = 0;
    int line_number = 0;
    char line[LINE_LENGTH];

    if (argc < 3) {
        printf("Usage: %s <file_name> <line_number> [<destination_file>]\n", argv[0]);
        return 1;
    }

    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("Failed to open the specified file\n");
        return 1;
    }

    line_number = atoi(argv[2]);

    if (get_line(fd, line, line_number, LINE_LENGTH, &read_bytes) == FAILURE) {
        perror("Failed to get the requested line\n");
        close(fd);
        return 1;
    }

    printf("%s", line);

    if (argc == 4) {
        if ((hd = open(argv[3], O_WRONLY)) == -1) {
            perror("Failed to open the destination file\n");
            close(fd);
            return 1;
        }

        reverse_lines(fd, hd);
        close(hd);
    }

    close(fd);

    return 0;
}