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
    int pipe_2_1[2], pipe_1_3[2];  // Pipes for C2->C1 and C1->C3 communication

    // Create pipes
    if (pipe(pipe_2_1) < 0 || pipe(pipe_1_3) < 0) {
        fprintf(stderr, "pipe() failed: %s\n", strerror(errno));
        return 1;
    }

    setbuf(stdout, NULL);

    cpid[0] = fork();
    if (cpid[0] < 0) {
        fprintf(stderr, "fork() 1 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[0]) { // CHILD-1
        char dummy;
        close(pipe_2_1[1]);  // Close write end of pipe_2_1
        close(pipe_1_3[0]);  // Close read end of pipe_1_3
        
        // Wait for C2
        read(pipe_2_1[0], &dummy, 1);
        printf("CHILD-1 (PID=%d) is running.\n", getpid());
        
        // Signal C3
        write(pipe_1_3[1], "x", 1);
        
        close(pipe_2_1[0]);
        close(pipe_1_3[1]);
        exit(0);
    }

    cpid[1] = fork();
    if (cpid[1] < 0) {
        fprintf(stderr, "fork() 2 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[1]) { // CHILD-2
        close(pipe_2_1[0]);  // Close read end
        close(pipe_1_3[0]);  // Close unused pipe
        close(pipe_1_3[1]);  // Close unused pipe
        
        printf("CHILD-2 (PID=%d) is running.\n", getpid());
        
        // Signal C1
        write(pipe_2_1[1], "x", 1);
        
        close(pipe_2_1[1]);
        exit(0);
    }
    
    cpid[2] = fork();
    if (cpid[2] < 0) {
        fprintf(stderr, "fork() 3 failed: %s\n", strerror(errno));
        return 0;
    }
    else if (0 == cpid[2]) { // CHILD-3
        char dummy;
        close(pipe_2_1[0]);  // Close unused pipe
        close(pipe_2_1[1]);  // Close unused pipe
        close(pipe_1_3[1]);  // Close write end
        
        // Wait for C1
        read(pipe_1_3[0], &dummy, 1);
        printf("CHILD-3 (PID=%d) is running.\n", getpid());
        
        close(pipe_1_3[0]);
        exit(0);
    }

    // Parent closes all pipe ends
    close(pipe_2_1[0]);
    close(pipe_2_1[1]);
    close(pipe_1_3[0]);
    close(pipe_1_3[1]);

    while ((ret = wait(NULL)) > 0)
    {
        printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), ret);
    }

    return 0;
}
