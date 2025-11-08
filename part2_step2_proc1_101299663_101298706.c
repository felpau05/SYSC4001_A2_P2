#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>

int main(void)
{
    const int multiple = 3;
    unsigned long long cycle = 0;
    unsigned long long value = 0;
    pid_t pid;

    printf("[PROC1] Starting. PID = %d\n", getpid());
    fflush(stdout);

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child branch: launch process 2 using exec()
        printf("[PROC1] Child about to exec Process 2. Child PID = %d\n",
               getpid());
        fflush(stdout);

        // Matches the actual compiled binary name.
        if (execl("./part2_step2_proc2", "part2_step2_proc2", (char *)NULL) == -1) {
            perror("execl failed");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_FAILURE);
    }

    // Parent branch: Process 1 logic 
    printf("[PROC1] Parent continues. PID = %d, Child PID (Process 2) = %d\n",
           getpid(), pid);
    fflush(stdout);

    while (1) {
        /*
         * Process 1: incrementing counter, printing cycle number,
         * and only marking multiples of 3.
         */
        printf("[PROC1] Cycle number: %llu", (unsigned long long)cycle);

        if (value % multiple == 0) {
            printf("  –  %llu is a multiple of %d\n",
                   (unsigned long long)value, multiple);
        } else {
            printf("\n");
        }

        cycle++;
        value++;           
        fflush(stdout);
        sleep(1);          // slow down 
    }

    return 0;
}
