#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int getUser(struct passwd *uPointer, const char *name) {
    errno = 0;

    // getpwnam may point to static area
    // *uPointer = getpwnam(name);
    struct passwd *bufUser = getpwnam(name);

    if (bufUser == NULL) {
        char buf[100];
        sprintf(buf, "An error has occurred while searching for user %s", name);
        perror(buf);
        return -1;
    }

    memcpy(uPointer, bufUser, sizeof(struct passwd));

    return 0;
}

int main(int argc, char **argv) {
    const char username[] = "logoeje";

    errno = 0;
    int retVal = 0;
    struct passwd *u1 = malloc(sizeof(struct passwd));
    struct passwd *u2 = malloc(sizeof(struct passwd));
    uid_t currentUid;

    if (argc != 3) {
        fprintf(stderr, "usage: ./program <file> <string>\n");
        return 1;
    }

    if (u1 == NULL || u2 == NULL) {
        char buf[100];
        sprintf(buf, "An error has occurred while allocating data");
        perror(buf);
        return 2;
    }

    do {
        if (getUser(u1, "root") != 0) {
            retVal = -1;
            break;
        }

        if (getUser(u2, username) != 0) {
            retVal = -1;
            break;
        }

        currentUid = getuid();

        if (currentUid != u1->pw_uid && currentUid != u2->pw_uid) {
            fprintf(stderr, "Access denied.\n");
            retVal = -2;
            break;
        }

        errno = 0;

        FILE* fo = fopen(argv[1], "a");

        if (fo == NULL) {
            char buf[100];
            sprintf(buf, "An error has occurred while opening the file");
            perror(buf);
            retVal = -3;
            break;
        }

        fprintf(fo, "%s\n", argv[2]);

        fclose(fo);

    } while(0);

    free(u1);
    free(u2);

    return retVal;
}