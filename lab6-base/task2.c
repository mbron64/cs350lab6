#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    pid_t cpid[3] = {0}; 
    int ret = 0;   

    setbuf(stdout, NULL);

    cpid[0] = fork();
    if (cpid[0] < 0) {
        fprintf(stderr, "fork() 1 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[0]) { // CHILD-1
        printf("CHILD-1 (PID=%d) is running.\n", getpid());
        exit(0);
    }

    cpid[1] = fork();
    if (cpid < 0) {
        fprintf(stderr, "fork() 2 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[1]) { // CHILD-2
        printf("CHILD-2 (PID=%d) is running.\n", getpid());
        exit(0);
    }
    
    cpid[2] = fork();
    if (cpid < 0) {
        fprintf(stderr, "fork() 3 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[2]) { // CHILD-3
        printf("CHILD-3 (PID=%d) is running.\n", getpid());
        exit(0);     
    }

    while ((ret = wait(NULL)) > 0)
    {
        printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), ret);
    }

    return 0;
}
