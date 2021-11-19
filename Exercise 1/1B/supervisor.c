#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "circbuffer.h"


#define SHM_NAME "/circbuff" 
#define SEM1_NAME "/sem1"
#define SEM2_NAME "/sem2"
#define SEM3_NAME "/sem3"
#define SEM4_NAME "/sem4"
#define MAX_DATA (50)

// SIGNAL HANDLING
volatile sig_atomic_t quit = 0;

void handle_signal(int signal);

int signal_setup(struct sigaction sa);


// SHARED MEMORY

int shm_setup ();


// SEMAPHORES

int sem_setup ();


int main (int argc, char *argv[]){
    sem_unlink(SEM1_NAME);
    sem_unlink(SEM2_NAME);
    sem_unlink(SEM3_NAME);
    sem_unlink(SEM4_NAME);
    shm_unlink(SHM_NAME);

    // setup shared memory
    // setup semaphores
    // setup circular buffer

    // wait for solution of generator
    // store and print best solution

    // setup signal handler
    struct sigaction sa;
    
    if (signal_setup(sa) == -1){
        fprintf(stderr, "[%s] ERROR: setting up signal handler\n %s\n", argv[0], strerror(errno));
        exit(1);
    }

    // setup shared memory object and map to memory
    int shmfd = shm_setup();
    if (shmfd == -1){
        fprintf(stderr, "[%s] ERROR: setting up shared memory\n %s\n", argv[0], strerror(errno));
        exit(1);
    }

    struct circ *circ;
    // circ = malloc(sizeof(struct circ));
    circ = mmap(0, sizeof(*circ), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (circ == MAP_FAILED) {
        fprintf(stderr, "[%s] ERROR: mapping shared memory\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    circ->quit2=false;
    circ->readpos = 0;
    circ->writepos = 0;


    // setup semaphores
    // sem_t sem1, sem2, sem3;
    sem_t *semfree = sem_open(SEM1_NAME, O_CREAT | O_EXCL, 0600, 8);
    sem_t *semused = sem_open(SEM2_NAME, O_CREAT | O_EXCL, 0600, 0);
    sem_t *semgen = sem_open(SEM3_NAME, O_CREAT | O_EXCL, 0600, 1);
    sem_t *semcomm = sem_open(SEM4_NAME, O_CREAT | O_EXCL, 0600, 0);

    if (semfree == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening 'free' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (semused == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening 'used' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (semgen == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening 'gen' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (semcomm == SEM_FAILED){
        fprintf(stderr, "[%s] ERROR: opening 'comm' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }


    // main loop terminates on SIGINT or SIGTERM
    int best = __INT_MAX__;
    int sol;

    while (!quit){
        if (sem_wait(semused) == -1){
            fprintf(stderr, "[%s] ERROR: wait for 'used' semaphore\n %s\n", argv[0], strerror(errno));
            exit(1);
        }
        sol = circ->data[circ->readpos];
        if (sol == 0){
            printf("The graph is 3-colourable!\n");
            quit=true;
        } else if (sol < best){
            best = sol;
            printf("Solution with %d edges: \n", best);
        }
        if (sem_post(semfree) == -1){
            fprintf(stderr, "[%s] ERROR: post for 'free' semaphore\n %s\n", argv[0], strerror(errno));
            exit(1);
        }
        circ->readpos+=1;
        circ->readpos%=8;
    }

    circ->quit2 = true;
    sem_wait(semcomm);

    // after loop inform generator to quit
    // wait for every generator to quit (something with semaphores)?
    // then fulfill closing process

    if (munmap(circ, sizeof(circ)) == -1){
        fprintf(stderr, "[%s] ERROR: unmapping circular buffer\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (close(shmfd) == -1){
        fprintf(stderr, "[%s] ERROR: closing shared memory\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (shm_unlink(SHM_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking shared memory\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_close(semgen) == -1){
        fprintf(stderr, "[%s] ERROR: closing 'gen' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_close(semused) == -1){
        fprintf(stderr, "[%s] ERROR: closing 'used' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_close(semfree) == -1){
        fprintf(stderr, "[%s] ERROR: closing 'free' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_close(semcomm) == -1){
        fprintf(stderr, "[%s] ERROR: closing 'comm' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_unlink(SEM3_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking 'gen' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_unlink(SEM2_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking 'used' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_unlink(SEM1_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking 'free' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_unlink(SEM4_NAME) == -1){
        fprintf(stderr, "[%s] ERROR: unlinking 'comm' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }

    exit(0);
}

// SIGNAL HANDLING

void handle_signal(int signal){
    printf("working!!!!");
    quit = 1;
}

int signal_setup(struct sigaction sa){
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    signal(SIGINT, handle_signal);
    sigaction(SIGINT, &sa, 0);
    sigaction(SIGTERM, &sa, 0);
    return 0;
}


// SHARED MEMORY

int shm_setup (){
    int shmfd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shmfd == -1) {
        return -1;
    }
    if (ftruncate(shmfd, sizeof(struct circ)) < 0){
        return -1;
    }
    return shmfd;
}


// SEMAPHORES


// GRAPH COLOURING & COMPUTATION

