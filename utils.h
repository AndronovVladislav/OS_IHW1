#ifndef IHW2_UTILS_H
#define IHW2_UTILS_H

#define SELLERS_AMOUNT 3
#define QUEUE_REGION "queue"
#define SEMAPHORES_REGION "semaphores"

#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct client {
    int sellers_to_visit[3];
} client;

void (*prev_handler)(int);

const char *sellers_names[] = {"/seller0", "/seller1", "/seller2"};
char pathname[] = "main.c";
char pathname_separated_files[] = "sellers.c";
sem_t *sellers[3] = {NULL, NULL, NULL};
sem_t *sellers_shm;

#endif //IHW2_UTILS_H
