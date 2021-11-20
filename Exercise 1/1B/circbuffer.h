#include <semaphore.h>

#define MAX_DATA (50)
#define SHM_NAME "/circbuff" 


struct circ {
    bool quit2;
    unsigned int writepos;
    unsigned int readpos;
    struct datas data[8];
};

struct vertex {
    int colour;
    char name;
};

struct edge {
    struct vertex v1, v2;
};

struct datas {
    struct edge edges[8];
    unsigned int count;
};