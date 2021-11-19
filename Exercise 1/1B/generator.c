#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <semaphore.h>
#include <stdbool.h>
#include "circbuffer.h"


#define SHM_NAME "/circbuff" 
#define SEM1_NAME "/sem1"
#define SEM2_NAME "/sem2"
#define SEM3_NAME "/sem3"
#define SEM4_NAME "/sem4"

struct vertex {
    int colour;
    char name;
};

struct edge {
    struct vertex v1, v2;
};

int generatevertices (struct vertex vertices[], int verticesize, struct edge edges[], int edgessize);

void generatecolours (struct vertex vertices[], struct edge edges[], int verticessize);

void attachcolours (struct vertex vertices[], int ind, struct edge edges[], int edgesize);

int generateSolution (struct edge edges[], int edgesize);


int main (int argc, char *argv[]){
struct edge edges[argc];

    for (int i = 1; i < argc; i++){
        edges[(i - 1)].v1.name = argv[i][0];
        edges[(i - 1)].v2.name = argv[i][2];
    }

    
    int shmfd;
    shmfd = shm_open(SHM_NAME, O_RDWR, S_IRWXU);
    if (shmfd == -1){
        fprintf(stderr, "[%s] ERROR: opening up shared memory\n %s\n", argv[0], strerror(errno));
        exit(1);
    }

    struct circ *circ;
    circ = malloc(sizeof(struct circ));
    circ = mmap(0, sizeof(circ), PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (circ == MAP_FAILED) {
        fprintf(stderr, "[%s] ERROR: mapping shared memory\n %s\n", argv[0], strerror(errno));
        exit(1);
    }

    sem_t *semfree = sem_open(SEM1_NAME, 0);
    sem_t *semused = sem_open(SEM2_NAME, 0);
    sem_t *semgen = sem_open(SEM3_NAME, 0);
    sem_t *semcomm = sem_open(SEM4_NAME, 0);

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


    // generate vertices and colouring of graph

    struct vertex vertices[argc];
    int ind = generatevertices(vertices, sizeof(vertices), edges, sizeof(edges));

    int bestSolution = __INT8_MAX__;

    while (circ->quit2 != true){


        // generate possible solution
        if (sem_wait(semgen) == -1){
            fprintf(stderr, "[%s] ERROR: wait for 'gen' semaphore\n %s\n", argv[0], strerror(errno));
            exit(1);
        }

        generatecolours(vertices, edges, ind);
        attachcolours(vertices, ind, edges, sizeof(edges));

        int sol = generateSolution(edges, sizeof(edges));

        // if (sem_wait(semgen) == -1){
        //     fprintf(stderr, "[%s] ERROR: wait for 'gen' semaphore\n %s\n", argv[0], strerror(errno));
        //     exit(1);
        // }
        if (sol < bestSolution){
            // write to circular buffer

            if (sem_wait(semfree) == -1){
                fprintf(stderr, "[%s] ERROR: wait for 'free' semaphore\n %s\n", argv[0], strerror(errno));
                exit(1);
            }
        
            circ->data[circ->writepos] = sol;
            bestSolution = sol;

            if (sem_post(semused) == -1){
                fprintf(stderr, "[%s] ERROR: post for 'used' semaphore\n %s\n", argv[0], strerror(errno));
                exit(1);
            }
            circ->writepos+=1;
            circ->writepos%=8;


        }   // else compute next solution

        if (sem_post(semgen) == -1){
            fprintf(stderr, "[%s] ERROR: post for 'gen' semaphore\n %s\n", argv[0], strerror(errno));
            exit(1);
        }
    }

    if (munmap(circ, sizeof(circ)) == -1){
        fprintf(stderr, "[%s] ERROR: unmapping circular buffer\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (close(shmfd) == -1){
        fprintf(stderr, "[%s] ERROR: closing shared memory\n %s\n", argv[0], strerror(errno));
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
    if (sem_post(semcomm) == -1){
        fprintf(stderr, "[%s] ERROR: post for 'used' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }
    if (sem_close(semcomm) == -1){
        fprintf(stderr, "[%s] ERROR: closing 'comm' semaphore\n %s\n", argv[0], strerror(errno));
        exit(1);
    }

    exit(0);
}

int generatevertices (struct vertex vertices[], int verticesize, struct edge edges[], int edgessize){

    int ind = 0;
    for (int i = 0; i < edgessize / sizeof(struct edge) - 1; i++){
        char first = (edges[i].v1).name;
        bool fst = true; 
        char second = (edges[i].v2).name;
        bool snd = true;


        for (int j = 0; j < verticesize / sizeof(struct vertex); j++){
            if (first == vertices[j].name){
                fst = false;
            }
            if (second == vertices[j].name){
                snd = false;
            }
        }
        if (fst == true){
            vertices[ind] = edges[i].v1;
            
            ind++;
        }
        if (snd == true){
            vertices[ind] = edges[i].v2;
            
            ind++;
        }

    }

    return ind;
}

void generatecolours (struct vertex vertices[], struct edge edges[], int verticessize){

    for (int i = 0; i < verticessize; i++){
        // 1 red, 2 green, 3 blue
        vertices[i].colour = rand() % 3;
    }
    
}

void attachcolours (struct vertex vertices[], int ind, struct edge edges[], int edgesize){

    for (int i = 0; i < ind; i++){
        char name = vertices[i].name;
        int colour = vertices[i].colour;

        for (int j = 0; j < edgesize / sizeof(struct edge); j++){
            if (name == edges[j].v1.name){
                edges[j].v1.colour = colour;
            }
            if (name == edges[j].v2.name){
                edges[j].v2.colour = colour;
            }
        }
    }

}


int generateSolution (struct edge edges[], int edgesize){
    int count = 0;
    for (int i = 0; i < edgesize / sizeof(struct edge) - 1; i++){
        if ((edges[i].v1).colour == (edges[i].v2).colour){
            (edges[i].v1).colour = -1;
            (edges[i].v2).colour = -1;
            count++;
        }
    }
    return count;
}