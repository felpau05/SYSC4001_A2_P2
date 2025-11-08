#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

struct shared_data {
    int multiple;
    int counter;
};

/* System V semctl() requires user code to define this union on many systems */
union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

static int sem_lock(int semid)
{
    struct sembuf op = {0, -1, 0};  // decrement semaphore 0
    return semop(semid, &op, 1);
}

static int sem_unlock(int semid)
{
    struct sembuf op = {0, +1, 0};  // increment semaphore 0
    return semop(semid, &op, 1);
}

int main(void)
{
    int shmid;
    int semid;
    struct shared_data *sh = NULL;
    pid_t pid;
    char shmid_str[32];
    char semid_str[32];
    int status;

    printf("[PROC1] Step 5 starting. PID = %d\n", getpid());
    fflush(stdout);

    // Create shared memory 
    shmid = shmget(IPC_PRIVATE, sizeof(struct shared_data), IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget failed");
        return EXIT_FAILURE;
    }

    sh = (struct shared_data *)shmat(shmid, NULL, 0);
    if (sh == (void *)-1) {
        perror("shmat failed");
        return EXIT_FAILURE;
    }

    // Create semaphore set with 1 semaphore 
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if (semid == -1) {
        perror("semget failed");
        shmdt(sh);
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    // Initialize semaphore to 1 (unlocked)
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL failed");
        shmdt(sh);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, arg);
        return EXIT_FAILURE;
    }

    // Initialize shared data under lock
    if (sem_lock(semid) == -1) {
        perror("sem_lock failed");
        return EXIT_FAILURE;
    }
    sh->multiple = 3;
    sh->counter  = 0;
    if (sem_unlock(semid) == -1) {
        perror("sem_unlock failed");
        return EXIT_FAILURE;
    }

    printf("[PROC1] Shared memory created: shmid=%d, multiple=3, counter=0, semid=%d\n",
           shmid, semid);
    fflush(stdout);

    // Prepare arguments to pass to Process 2 (shmid, semid)
    snprintf(shmid_str, sizeof(shmid_str), "%d", shmid);
    snprintf(semid_str, sizeof(semid_str), "%d", semid);

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        shmdt(sh);
        shmctl(shmid, IPC_RMID, NULL);
        union semun dummy;
        semctl(semid, 0, IPC_RMID, dummy);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child: exec Process 2 with shmid and semid
        printf("[PROC1] Child about to exec Process 2 (shmid=%s, semid=%s). PID=%d\n",
               shmid_str, semid_str, getpid());
        fflush(stdout);

        if (execl("./part2_step5_proc2_101299663_101298706",
                  "part2_step5_proc2_101299663_101298706",
                  shmid_str,
                  semid_str,
                  (char *)NULL) == -1) {
            perror("execl failed");
            _exit(EXIT_FAILURE);
        }
        _exit(EXIT_FAILURE);  // should not reach here
    }

    // Parent: increment shared counter under semaphore until > 500 
    printf("[PROC1] Parent continues. PID=%d, Child PID=%d\n", getpid(), pid);
    fflush(stdout);

    while (1) {
        int local_counter;

        if (sem_lock(semid) == -1) {
            perror("sem_lock failed");
            break;
        }

        local_counter = sh->counter;
        printf("[PROC1] Counter in shared memory (before inc) = %d\n", local_counter);

        sh->counter = local_counter + 1;
        local_counter = sh->counter;

        if (sem_unlock(semid) == -1) {
            perror("sem_unlock failed");
            break;
        }

        printf("[PROC1] Counter in shared memory (after inc) = %d\n", local_counter);
        fflush(stdout);

        if (local_counter > 500) {
            printf("[PROC1] Counter > 500, stopping loop.\n");
            fflush(stdout);
            break;
        }

        sleep(1);
    }

    // wait for child to finish
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
    } else {
        printf("[PROC1] Child %d finished. Cleaning up shared memory and semaphore.\n", pid);
    }

    // Dettach and remove shared memory
    if (shmdt(sh) == -1) {
        perror("shmdt failed");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl(IPC_RMID) failed");
    }

    // Remove semaphore set
    union semun dummy;
    if (semctl(semid, 0, IPC_RMID, dummy) == -1) {
        perror("semctl(IPC_RMID) failed");
    }

    printf("[PROC1] Exiting.\n");
    fflush(stdout);
    return 0;
}