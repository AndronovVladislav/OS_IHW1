#include "../utils.h"

void close_all(int nsig) {
    key_t key;

    if ((key = ftok(pathname_separated_files, 0)) < 0) {
        perror("ftok");
        exit(-1);
    }

    int semid;
    if ((semid = semget(key, SELLERS_AMOUNT, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        if ((semid = semget(key, SELLERS_AMOUNT, 0)) < 0) {
            printf("Can\'t connect to semaphore\n");
            exit(-1);
        }
        printf("Connected to Shared Semaphores for delete\n");
    }

    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        semctl(semid, i, IPC_RMID);
        printf("%d semaphore was deleted\n", i);
    }

    exit(0);
}

int main() {
    prev_handler = signal(SIGINT, close_all);

    key_t key;
    if ((key = ftok(pathname_separated_files, 0)) < 0) {
        perror("ftok");
        exit(-1);
    }

    int semid;
    if ((semid = semget(key, SELLERS_AMOUNT, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        if ((semid = semget(key, SELLERS_AMOUNT, 0)) < 0) {
            printf("Can\'t connect to semaphore\n");
            exit(-1);
        }
        printf("Connected to Shared Semaphores\n");
    } else {
        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            semctl(semid, i, SETVAL, 0);
        }
        printf("New Shared Semaphores\n");
    }

    int shmid;
    int *sellers_busyness;
    if ((shmid = shmget(key, sizeof(int) * SELLERS_AMOUNT, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        if ((shmid = shmget(key, sizeof(int) * SELLERS_AMOUNT, 0)) < 0) {
            printf("Can\'t connect to shared memory\n");
            exit(-1);
        }
        sellers_busyness = (int*) shmat(shmid, NULL, 0);
        printf("Connected to Shared Memory\n");
    } else {
        sellers_busyness = (int*) shmat(shmid, NULL, 0);
        printf("New Shared Memory\n");
    }

    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        sellers_busyness[i] = 0;
    }

    pid_t chpids[3];
    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        if ((chpids[i] = fork()) == -1) {
            printf("I can't create seller process :c\n");
            exit(-1);
        } else if (chpids[i] == 0) {
            // получить доступ к памяти
            signal(SIGINT, prev_handler);
            int seller_is_busy;
            struct sembuf post_operation = { i, 1, 0 };
            while (1) {
                if (sellers_busyness[i] == 2) {
                    break;
                }

                seller_is_busy = semctl(semid, i, GETVAL);
                if (!seller_is_busy && !(sellers_busyness[i] == 0 || sellers_busyness[i] == 2)) {
                    usleep((float) (rand() % 1500 + 500) * 100);
                    if (semop(semid, &post_operation, 1) < 0) {
                        perror("sem_post");
                    }
                }
            }
            exit(0);
        }
    }

    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        waitpid(chpids[i], NULL, 0);
    }

    close_all(0);
    return 0;
}
