#include <semaphore.h>

#define MAX_DATA (50)
#define SHM_NAME "/circbuff" 


struct circ {
    bool quit2;
    unsigned int writepos;
    unsigned int readpos;
    unsigned int data[8];
};