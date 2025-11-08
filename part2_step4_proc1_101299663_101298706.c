#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

struct shared_data {
    int multiple;   
    int counter; 
};

int main(void)
{
    int shmid;
    struct shared_data *sh = NULL;
    pid_t pid;
    char shmid_str[32];
    int status;

    printf("[PROC1] Step 4 starting. PID = %d\n", getpid());
    fflush(stdout);

    // Create shared memory segment
    shmid = shmget(IPC_PRIVATE, sizeof(struct shared_data), IPC_CREAT | 0600);
    if (shmid == -1) {
        perror("shmget failed");
        return EXIT_FAILURE;
    }

    // Attach to it
    sh = (struct shared_data *)shmat(shmid, NULL, 0);
    if (sh == (void *)-1) {
        perror("shmat failed");
        return EXIT_FAILURE;
    }

    // Initialize shared variables
    sh->multiple = 3;
    sh->counter  = 0;

    printf("[PROC1] Shared memory created: shmid=%d, multiple=%d, counter=%d\n",
           shmid, sh->multiple, sh->counter);
    fflush(stdout);

    // Convert shmid to string to pass to child via exec()
    snprintf(shmid_str, sizeof(shmid_str), "%d", shmid);

    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        // clean up shm
        shmdt(sh);
        shmctl(shmid, IPC_RMID, NULL);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // ----- Child: become Process 2 with exec -----
        printf("[PROC1] Child about to exec Process 2 (shmid=%s). PID=%d\n",
               shmid_str, getpid());
        fflush(stdout);

        if (execl("./part2_step4_proc2", "part2_step4_proc2",
                  shmid_str, (char *)NULL) == -1) {
            perror("execl failed");
            _exit(EXIT_FAILURE);
        }
        _exit(EXIT_FAILURE);  
    }

    // ----- Parent: increment shared counter until > 500 -----
    printf("[PROC1] Parent continues. PID=%d, Child PID=%d\n", getpid(), pid);
    fflush(stdout);

    while (1) {
        printf("[PROC1] Counter in shared memory = %d (multiple=%d)\n",
               sh->counter, sh->multiple);
        fflush(stdout);

        sh->counter++;

        if (sh->counter > 500) {
            printf("[PROC1] Counter > 500, stopping loop.\n");
            fflush(stdout);
            break;
        }

        sleep(1);
    }

    // Wait for child to finish
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid failed");
    } else {
        printf("[PROC1] Child %d finished. Cleaning up shared memory.\n", pid);
    }

    // Detach and remove shared memory
    if (shmdt(sh) == -1) {
        perror("shmdt failed");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl(IPC_RMID) failed");
    }

    printf("[PROC1] Exiting.\n");
    fflush(stdout);
    return 0;
}
