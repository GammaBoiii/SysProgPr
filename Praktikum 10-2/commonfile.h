#pragma once

#define MEMSZE 256
#define TFILE "target.dat"

typedef struct {
    unsigned char buffer[MEMSZE];
    int in_pos;
    int out_pos;
} RingBuffer;

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
