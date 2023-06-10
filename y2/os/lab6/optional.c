#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int pid, steps, skip;
  
    if (argc != 3) {
        printf("Usage: %s <loop_steps> <skip_step>", argv[0]);
        exit(1);
    }
    
    sscanf(argv[1], "%d", &steps);
    sscanf(argv[2], "%d", &skip);

    for (int i=1; i<=steps; i++) {
        pid = fork();
        if (pid == 0 && i % skip == 0)
            break;
    }
}