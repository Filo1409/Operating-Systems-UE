#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "test.h"

int main(int argc, char* argv[]){
    // check if SYNOPSIS is matching
    if (argc != 1){
        fprintf(stderr, "[%s] ERROR: wrong number of arguments!\nSYNOPSIS [test1]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // semaphore setup
    if (sem_unlink(SEM1_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking sempahore %s!\n%s\n", argv[0], SEM1_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_unlink(SEM2_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking semaphore %s!\n%s\n", argv[0], SEM2_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    //shared memory setup
    if (shm_unlink(SHM_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking shared memory %s!\n%s\n", argv[0], SHM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int *shm = shm_open(SHM_NAME, O_CREAT | O_EXCL, O_RDWR);
    if (shm_unlink(SHM_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: opening shared memory %s!\n%s\n", argv[0], SHM_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm, 8) == -1){
        fprintf(stderr, "[%s] ERROR: allocating space for shared memory %s!\n%s\n", argv[0], SHM_NAME, 
            strerror(errno));
        exit(EXIT_FAILURE);
    }


    sem_t *sem1test = sem_open(SEM1_NAME, O_CREAT | O_EXCL, 0660, 0);
    sem_t *sem2test = sem_open(SEM2_NAME, O_CREAT | O_EXCL, 0660, 0);
    if (sem1test == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening up semaphore %s!\n%s\n", argv[0], SEM1_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem2test == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening up semaphore %s!\n%s\n", argv[0], SEM2_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("[%s] program running!\n", argv[0]);

    // make sure test1 takes longer than test2
    sem_post(sem1test);
    sem_wait(sem2test);
    printf("got here\n");

    sem_close(sem1test);
    sem_close(sem2test);
    sem_unlink(SEM1_NAME);
    sem_unlink(SEM2_NAME);
}