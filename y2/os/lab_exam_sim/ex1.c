#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {

    int wStatus = 1;
    pid_t pid = -1;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ls-arg>\n", argv[0]);
        return 1;
    }

    pid = fork();
    if (pid < 0) {

        fprintf(stderr, "Child process creation failed: %s\n", strerror(errno));
        return 1;

    } else if (pid == 0) {

        printf("[Child] pid = %d ppid = %d cpid = %d\n", getpid(), getppid(), pid);
        wStatus = execlp("ls", "ls", argv[1], (char*) NULL);
        printf("Process execution has failed: %s\n", strerror(errno));
        return wStatus;

    } else {

        printf("[Parent] pid = %d ppid = %d cpid = %d\n", getpid(), getppid(), pid);
        waitpid(pid, &wStatus, 0);
        printf("[Parent] The child has exited with code %d\n", wStatus);

    }

    return 0;
}