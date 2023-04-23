#include "../utils.h"

void close_all(int nsig) {
    int shmid;
    if ((shmid = shm_open(QUEUE_REGION, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
        perror("shm_open");
        exit(-1);
    }

    int *forming_queue = mmap(0, sizeof(int) * SELLERS_AMOUNT, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        forming_queue[i] = 2;
    }

    close(shmid);
    shm_unlink(QUEUE_REGION);

    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        if (sem_close(sellers[i]) < 0) {
            perror("sem_close");
        }

        if (sem_unlink(sellers_names[i]) < 0) {
            perror("sem_unlink");
        }
    }

    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Incorrect amount of clients!\n");
        return 0;
    }

    prev_handler = signal(SIGINT, close_all);
    for (int i = 0; i < SELLERS_AMOUNT; ++i) {
        if ((sellers[i] = sem_open(sellers_names[i], O_CREAT, 0777, 0)) == SEM_FAILED) {
            perror("sem_open");
        }
    }

    int shmid;
    if ((shmid = shm_open(QUEUE_REGION, O_CREAT | O_RDWR, S_IRWXU)) == -1) {
        perror("shm_open");
        exit(-1);
    }

    if (ftruncate(shmid, sizeof(int) * SELLERS_AMOUNT) == -1) {
        perror("ftruncate");
        exit(-1);
    }

    pid_t chpid;
    if ((chpid = fork()) == -1) {
        printf("I can't create customers-creator process :c\n");
        exit(-1);
    } else if (chpid == 0) {
        signal(SIGINT, prev_handler);
        int *forming_queue = mmap(0, sizeof(int) * SELLERS_AMOUNT, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            forming_queue[i] = 0;
        }

        usleep(1000000);
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
                    if (current_client.sellers_to_visit[j] == 1) {
                        forming_queue[j] = 1;
                        printf("I'm %d client and now I'm waiting for service by seller%d\n", i, j);

                        if (sem_wait(sellers[j]) < 0) {
                            perror("sem_wait");
                        }

                        printf("I'm %d client and I have been served by seller%d\n", i, j);
                    }
                }
                exit(0);
            }
        }

        for (int i = 0; i < atoi(argv[1]); ++i) {
            waitpid(chpids[i], NULL, 0);
        }

        free(chpids);

        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            forming_queue[i] = 2;
        }
    } else {
        signal(SIGINT, prev_handler);
        pid_t chpids[3];
        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            if ((chpids[i] = fork()) == -1) {
                printf("I can't create seller process :c\n");
                exit(-1);
            } else if (chpids[i] == 0) {
                int *queue_empty = mmap(0, sizeof(int) * SELLERS_AMOUNT, PROT_WRITE | PROT_READ, MAP_SHARED, shmid, 0);
                int seller_is_busy;
                while (1) {
                    if (queue_empty[i] == 2) {
                        break;
                    }

                    sem_getvalue(sellers[i], &seller_is_busy);
                    if (!seller_is_busy && queue_empty[i] == 1) {
                        usleep((float) (rand() % 1501 + 500) * 100);
                        if (sem_post(sellers[i]) < 0) {
                            perror("sem_post");
                        }
                    }
                }
                return 0;
            }
        }

        for (int i = 0; i < SELLERS_AMOUNT; ++i) {
            waitpid(chpids[i], NULL, 0);
        }

        close_all(0);
    }

    return 0;
}
