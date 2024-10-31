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
    // Create two pipes for synchronization
    int pipe1[2], pipe2[2];
    char buf[2];
    
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        fprintf(stderr, "pipe creation failed: %s\n", strerror(errno));
        return 1;
    }

    setbuf(stdout, NULL);

    cpid[0] = fork();
    if (cpid[0] < 0) {
        fprintf(stderr, "fork() 1 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[0]) { // CHILD-1 (runs last)
        close(pipe1[1]); // Close unused write ends
        close(pipe2[1]);
        read(pipe2[0], buf, 1); // Wait for C2
        printf("CHILD-1 (PID=%d) is running.\n", getpid());
        close(pipe1[0]);
        close(pipe2[0]);
        exit(0);
    }

    cpid[1] = fork();
    if (cpid[1] < 0) {
        fprintf(stderr, "fork() 2 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[1]) { // CHILD-2 (runs second)
        close(pipe1[1]); // Close unused write end
        close(pipe2[0]); // Close unused read end
        read(pipe1[0], buf, 1); // Wait for C3
        printf("CHILD-2 (PID=%d) is running.\n", getpid());
        write(pipe2[1], "x", 1); // Signal C1
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }
    
    cpid[2] = fork();
    if (cpid[2] < 0) {
        fprintf(stderr, "fork() 3 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[2]) { // CHILD-3 (runs first)
        close(pipe1[0]); // Close unused read end
        close(pipe2[0]); // Close unused read end
        close(pipe2[1]); // Close unused write end
        printf("CHILD-3 (PID=%d) is running.\n", getpid());
        write(pipe1[1], "x", 1); // Signal C2
        close(pipe1[1]);
        exit(0);     
    }

    // Parent process should close all pipe ends
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    while ((ret = wait(NULL)) > 0)
    {
        printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), ret);
    }

    return 0;
}
