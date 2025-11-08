#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/wait.h>  // fine for Linux (WSL)


int main(void)
{
    pid_t pid;
    int status;

    printf("[PROC1] Starting. PID = %d\n", getpid());
    fflush(stdout);

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child -> become Process 2 via exec() 
        printf("[PROC1] Child about to exec Process 2. Child PID = %d\n",
               getpid());
        fflush(stdout);

        if (execl("./part2_step3_proc2", "part2_step3_proc2", (char *)NULL) == -1) {
            perror("execl failed");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_FAILURE);
    }

    // Parent branch: wait for Process 2 
    printf("[PROC1] Parent continues. PID = %d, waiting for child PID = %d\n",
           getpid(), pid);
    fflush(stdout);

    // wait for specific child
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("[PROC1] Child %d exited with status %d. Process 1 exiting too.\n",
               pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[PROC1] Child %d was terminated by signal %d. Process 1 exiting.\n",
               pid, WTERMSIG(status));
    } else {
        printf("[PROC1] Child %d finished (unknown state). Process 1 exiting.\n", pid);
    }

    fflush(stdout);
    return 0;
}
