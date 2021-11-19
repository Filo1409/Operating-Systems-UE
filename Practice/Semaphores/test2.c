#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "test.h"

int main(int argc, char* argv[]){
    if (argc != 1){
        fprintf(stderr, "[%s] ERROR: wrong number of arguments!\nSYNOPSIS [test2]\n", 
        argv[0]);
        exit(EXIT_FAILURE);
    }

    sem_t *sem1test = sem_open(SEM1_NAME, 0);
    sem_t *sem2test = sem_open(SEM2_NAME, 0);
    if (sem1test == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening up semaphore %s!\n%s\n", argv[0], SEM1_NAME, 
        strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem2test == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening up semaphore %s!\n%s\n", argv[0], SEM2_NAME, 
        strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_wait(sem1test);
    printf("[%s] program running!\n", argv[0]);
    sem_post(sem2test);

    sem_close(sem1test);
    sem_close(sem2test);
}