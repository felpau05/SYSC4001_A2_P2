#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

struct shared_data {
    int multiple;
    int counter;
};

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

static int sem_lock(int semid)
{
    struct sembuf op = {0, -1, 0};
    return semop(semid, &op, 1);
}

static int sem_unlock(int semid)
{
    struct sembuf op = {0, +1, 0};
    return semop(semid, &op, 1);
}

int main(int argc, char *argv[])
{
    int shmid;
    int semid;
    struct shared_data *sh = NULL;

    if (argc < 3) {
        fprintf(stderr, "[PROC2] Usage: %s <shmid> <semid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    shmid = atoi(argv[1]);
    semid = atoi(argv[2]);

    if (shmid <= 0 || semid <= 0) {
        fprintf(stderr, "[PROC2] Invalid shmid or semid.\n");
        return EXIT_FAILURE;
    }

    printf("[PROC2] Starting. PID=%d, attaching to shmid=%d, semid=%d\n",
           getpid(), shmid, semid);
    fflush(stdout);

    sh = (struct shared_data *)shmat(shmid, NULL, 0);
    if (sh == (void *)-1) {
        perror("[PROC2] shmat failed");
        return EXIT_FAILURE;
    }

    // Wait until counter > 100 (reads protected by semaphore)
    while (1) {
        int c;

        if (sem_lock(semid) == -1) {
            perror("[PROC2] sem_lock failed");
            break;
        }

        c = sh->counter;

        if (sem_unlock(semid) == -1) {
            perror("[PROC2] sem_unlock failed");
            break;
        }

        if (c > 100) {
            printf("[PROC2] Counter now %d (>100). Starting to react.\n", c);
            fflush(stdout);
            break;
        }

        printf("[PROC2] Waiting, counter=%d (need >100)\n", c);
        fflush(stdout);
        sleep(1);
    }

    // react to sharred memory until counter > 500
    while (1) {
        int c, m;

        if (sem_lock(semid) == -1) {
            perror("[PROC2] sem_lock failed");
            break;
        }

        c = sh->counter;
        m = sh->multiple;

        if (sem_unlock(semid) == -1) {
            perror("[PROC2] sem_unlock failed");
            break;
        }

        if (c > 500) {
            printf("[PROC2] Counter > 500 (c=%d). Exiting.\n", c);
            fflush(stdout);
            break;
        }

        printf("[PROC2] Seen counter=%d, multiple=%d", c, m);
        if (m != 0 && (c % m) == 0) {
            printf("  –  %d is a multiple of %d\n", c, m);
        } else {
            printf("\n");
        }

        fflush(stdout);
        sleep(1);
    }

    if (shmdt(sh) == -1) {
        perror("[PROC2] shmdt failed");
    }

    return 0;
}
