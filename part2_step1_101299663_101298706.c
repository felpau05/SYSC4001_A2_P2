#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <sys/types.h>


int main(void)
{
    pid_t pid;
    unsigned long long counter_parent = 0;
    unsigned long long counter_child  = 0;

    printf("Parent starting. PID = %d\n", getpid());
    fflush(stdout);

    pid = fork();

    if (pid < 0) {
        perror("forrk failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child process 
        printf("Child started. PID = %d, Parent PID = %d\n",
               getpid(), getppid());
        fflush(stdout);

        while (1) {
            counter_child++;
            printf("[CHILD ] PID=%d  counter_child=%llu\n",
                   getpid(),
                   (unsigned long long)counter_child);
            fflush(stdout);
            sleep(1);  // slow down output 
        }
    } else {
        // Parent process 
        printf("Parent continues. PID = %d, Child PID = %d\n",
               getpid(), pid);
        fflush(stdout);

        while (1) {
            counter_parent++;
            printf("[PARENT] PID=%d  counter_parent=%llu\n",
                   getpid(),
                   (unsigned long long)counter_parent);
            fflush(stdout);
            sleep(1);  // slow down output
        }
    }

    return 0;
}
