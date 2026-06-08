#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MEMSZE 256
#define TFILE "target.dat"

typedef struct {
    unsigned char buffer[MEMSZE];
    int in_pos;
    int out_pos;
} RingBuffer;

/*typedef struct { // Dient dazu, in einem signalhandler (der in dieser .h liegt) shmem und semid sauber zu entfernen.
        int shmid;
        int semid;
} Cleaner;*/
 struct Cleaner {
        int shmid;
        int semid;
};


#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union automatisch in sem.h inclusive*/
#else 
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };
#endif
