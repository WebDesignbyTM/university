#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    pid_t childPid = fork();
    pid_t selfPid = getpid();
    int chidStatus;__

    switch (childPid) {
    case -1:
        // error case
        fprintf(2, "[Process %d] Cannot create a new child\n", selfPid);
        exit(1);
    case 0:
        // child 
        //... does what it is requested ...
        break;
    default:
        // parent
        // ... does what it is requested ...
        printf("[Process %d] Created child process %d\n", selfPid, childPid);
        waitpid(childPid, &childStatus, 0);
    } 
}