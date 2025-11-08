#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct shared_data {
    int multiple;
    int counter;
};

int main(int argc, char *argv[])
{
    int shmid;
    struct shared_data *sh = NULL;

    if (argc < 2) {
        fprintf(stderr, "[PROC2] Usage: %s <shmid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    shmid = atoi(argv[1]);
    if (shmid <= 0) {
        fprintf(stderr, "[PROC2] Invalid shmid: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("[PROC2] Starting. PID=%d, attaching to shmid=%d\n",
           getpid(), shmid);
    fflush(stdout);

    sh = (struct shared_data *)shmat(shmid, NULL, 0);
    if (sh == (void *)-1) {
        perror("[PROC2] shmat failed");
        return EXIT_FAILURE;
    }

    // Wait untill counter > 100
    while (sh->counter <= 100) {
        printf("[PROC2] Waiting, counter=%d (need >100)\n", sh->counter);
        fflush(stdout);
        sleep(1);
    }

    printf("[PROC2] Counter now %d (>100). Reacting to shared memory.\n",
           sh->counter);
    fflush(stdout);

    // Shared memory,, until counter > 500
    while (sh->counter <= 500) {
        int c = sh->counter;
        int m = sh->multiple;

        printf("[PROC2] Seen counter=%d, multiple=%d", c, m);
        if (m != 0 && c % m == 0) {
            printf("  –  %d is a multiple of %d\n", c, m);
        } else {
            printf("\n");
        }

        fflush(stdout);
        sleep(1);
    }

    printf("[PROC2] Counter > 500, exiting. Final counter=%d\n", sh->counter);
    fflush(stdout);

    if (shmdt(sh) == -1) {
        perror("[PROC2] shmdt failed");
    }

    return 0;
}
