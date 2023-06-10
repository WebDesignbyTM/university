#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#define BUFFER_SIZE 64
#define SUCCESS 0
#define FAILURE 1
#define RW_PERM S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

#pragma pack(push, 1)

typedef struct _OPERATION {
    int no1;
    int no2;
    char operator;
} OPERATION;

#pragma pack(pop)

int execute(OPERATION *op) {

    if (op == NULL) {
        perror("Invalid operation pointer\n");
        return -1;
    }

    switch (op->operator) {
        case '-':
            return op->no1 - op->no2;

        case '*':
            return op->no1 * op->no2;

        default:
            return op->no1 + op->no2;
    }
}

int write_opreation(int fd, OPERATION *op) {

    if (op == NULL) {
        perror("Invalid operation pointer\n");
        return FAILURE;
    }

    if (write(fd, op, sizeof(OPERATION)) == -1) {
        perror("Failed to write operation to file\n");
        return FAILURE;
    }

    return SUCCESS;
}

int read_operation(int fd, OPERATION *op) {

    if (op == NULL) {
        perror("Invalid operation pointer\n");
        return FAILURE;
    }

    if (read(fd, op, sizeof(OPERATION)) != sizeof(OPERATION)) {
        perror("Failed to read operation from file\n");
        return FAILURE;
    }

    return SUCCESS;
}

// Am ales sa adaug un parametru functiei pentru ca niciuna dintre
// celelalte functii nu a avut access direct la parametrii programului
// si asa ar fi cel mai potrivit conform paradigmei programarii modulare
int perform_operations(int fd, char *fname) {

    char buf[BUFFER_SIZE] = { 0 };
    int hd;
    OPERATION op;
    off_t curr_offset = lseek(fd, 0, SEEK_CUR);
    off_t end_offset = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, curr_offset);

    if ((hd = open(fname, O_WRONLY | O_APPEND | O_CREAT, RW_PERM)) == -1) {
        perror("Failed to open output file\n");
    }

    // Lookahead-ul din conditia de while e necesar pentru a evita
    // un perror din read_operation de la finalul fisierului
    while (lseek(fd, 0, SEEK_CUR) != end_offset) {
        if (read_operation(fd, &op) == FAILURE) {
            close(hd);
            return FAILURE;
        }

        sprintf(buf, "%d %c %d = %d\n", op.no1, op.operator, op.no2, execute(&op));
        if (write(hd, buf, strlen(buf)) == -1) {
            perror("Failed to write to destination file\n");
            close(hd);
            return FAILURE;
        }
        memset(buf, 0, BUFFER_SIZE);
    }

    close(hd);

    return SUCCESS;
}

// Mi-am dat seama ca programele pe care le scriu sunt exagerat de robuste din
// cauza tuturor validarilor pe care le fac; daca sunt enervant de urmarit la 
// corectare, scrie-mi in feedback si sar peste verificarile redundante (i.e.
// e destul de improbabil ca parametrii functiilor pe care le scriu sa fie 
// invalizi, avand in vedere ca nu le apelez decat eu)
int main(int argc, char **argv) {
    int fd;
    OPERATION op1, op2, op3;

    umask(0000);

    switch (argc) {

    case 2:

        if ((fd = open(argv[1], O_WRONLY | O_CREAT, RW_PERM)) == -1) {
            perror("Failed to open destination file\n");
            return 1;
        }

        op1.no1 = 2;
        op1.no2 = 3;
        op1.operator = '*';
        op2.no1 = 0xff52ff02;
        op2.no2 = 0xad00fe;
        op2.operator = '+';
        op3.no1 = 0b110;
        op3.no2 = 01;
        op3.operator = '-';

        write_opreation(fd, &op1);
        write_opreation(fd, &op2);
        write_opreation(fd, &op3);

        close(fd);

        break;

    case 3:

        if ((fd = open(argv[2], O_RDONLY)) == -1) {
            perror("Failed to open source file\n");
            return 1;
        }

        perform_operations(fd, argv[1]);

        close(fd);

        break;

    default:
        printf("Usage: %s <destination_file> [<source_file>]\n", argv[0]);
        return 1;
    }

    return 0;
}