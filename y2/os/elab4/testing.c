#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
/* 
    // The mask persists for the rest of the process
    // but it doesn't affect chmod calls
    umask(022);

    if (creat("temp1", 0666) == -1) {
        perror("could not create temp1\n");
    }

    chmod("temp1", 0664);

    // umask can be used again to reset the process mask
    umask(000);

    if (creat("temp2", 0666) == -1) {
        perror("could not create temp2\n");
    }

    system("ls -l temp*");
    system("rm temp*");
*/

    
    return 0;
}