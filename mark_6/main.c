#include "../utils.h"

void close_all(int nsig) {
    key_t key;

    if ((key = ftok(pathname, 0)) < 0) {
        perror("ftok");
        exit(-1);
    }

    int shmid;
    int *sellers_busyness;
    if ((shmid = shmget(key, sizeof(int) * SELLERS_AMOUNT, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        if ((shmid = shmget(key, sizeof(int) * SELLERS_AMOUNT, 0)) < 0) {
            printf("Can\'t connect to shared memory for delete\n");
            exit(-1);
        }
        sellers_busyness = (int*) shmat(shmid, NULL, 0);
        printf("Connected to Shared Memory for delete\n");
    } else {
        sellers_busyness = (int*) shmat(shmid, NULL, 0);
        printf("New Shared Memory for delete\n");
    }

    if (shmdt(sellers_busyness) < 0) {
        perror("shmdt");
    }
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl");
    }
    printf("Shared memory was deleted\n");

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

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Incorrect amount of clients!\n");
        return 0;
    }
    prev_handler = signal(SIGINT, close_all);

    key_t key;
    if ((key = ftok(pathname, 0)) < 0) {
        perror("ftok");
        exit(-1);
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

    pid_t chpid;
    if ((chpid = fork()) == -1) {
        printf("I can't create customers-creator process :c\n");
        exit(-1);
    } else if (chpid == 0) {
        signal(SIGINT, prev_handler);

        pid_t *chpids = malloc(atoi(argv[1]) * sizeof(pid_t));
        for (int i = 0; i < atoi(argv[1]); ++i) {
            client current_client;
            srand(time(NULL));
            current_client.sellers_to_visit[rand() % 3] = 1;
            if ((chpids[i] = fork()) == -1) {
                printf("I can't create customer process :c\n");
                exit(-1);
            } else if (chpids[i] == 0) {
                for (int j = 0; j < SELLERS_AMOUNT; ++j) {
                    current_client.sellers_to_visit[j] = current_client.sellers_to_visit[j] == 1 ? 1 : rand() % 2;
//                    printf("%d ", current_client.sellers_to_visit[j]); // для проверки того, что действительно могут
//                    генерироваться кофигурации, в которых ни один посетитель не идёт ни к одному продавцу(если не
//                    использовать трюк в 105 и 111 строках)
                    if (current_client.sellers_to_visit[j] == 1) {
                        sellers_busyness[j] = 1;
                        printf("I'm %d client and now I'm waiting for service by seller%d\n", i, j);

                        struct sembuf wait_operation = { j, -1, 0 };
                        if (semop(semid, &wait_operation, 1) < 0) {
                            perror("sem_wait");
                        }

                        printf("I'm %d client and I have been served by seller%d\n", i, j);
                    }
                }
//                printf("\n"); // приятный перевод строки для описанной выше проверки
                exit(0);
            }
        }

        for (int i = 0; i < atoi(argv[1]); ++i) {
            waitpid(chpids[i], NULL, 0);
        }

        free(chpids);

        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            sellers_busyness[i] = 2;
        }
    } else {
        signal(SIGINT, prev_handler);
        pid_t chpids[3];
        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            if ((chpids[i] = fork()) == -1) {
                printf("I can't create seller process :c\n");
                exit(-1);
            } else if (chpids[i] == 0) {
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
    }

    return 0;
}