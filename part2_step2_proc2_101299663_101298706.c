#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>

int main(void)
{
    const int multiple = 3;
    long long value = 0;             // will be decremented 
    unsigned long long cycle = 0;

    printf("[PROC2] Started. PID = %d (launched via exec)\n", getpid());
    fflush(stdout);

    while (1) {
        value--;   

        printf("[PROC2] Cycle number: %llu",
               (unsigned long long)cycle);

        if (value % multiple == 0) {
            printf("  –  %lld is a multiple of %d (decrementing)\n",
                   value, multiple);
        } else {
            printf("\n");
        }

        cycle++;
        fflush(stdout);
        sleep(1);   // slow down
    }

    return 0;
}
