#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>

#define MEMSZE 256
#define TFILE "target.dat"

typedef struct {
    char buffer[MEMSZE];
    int in_pos;
    int out_pos;
} RingBuffer;

int timeout_triggered = 0;

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    /* union automatisch in sem.h inclusive*/
#else 
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };
#endif

// hilfsmethode für putsem
void P(int semid, int semnum) {
    struct sembuf op = {
        semnum,
        -1,
        0
     };
    if(semop(semid, &op, 1) < 0) {
        perror("semop P");
        exit(EXIT_FAILURE);
    }
}

//hilfsmethode für putsem, aber mit absicherung vor anderen unterbrechungen/signalen
//erstaz für P() bei dem 3Sekunden Timer
int P_safe(int semid, int semnum) {
    struct sembuf op = { semnum, -1, 0 };
    
    while (semop(semid, &op, 1) < 0) { //whileschleife startet semop hier neu, wenn es unterbrochen wurde (gibt dann nämlich -1 aus)
        if (errno == EINTR) {
            if(timeout_triggered) {
                return 0; //timer ausgelöst, dann darf es erstmal weiter gehen.
            } else {
                continue; //kein timer signal, also erstmal weiter warten
            }
        }
        perror("semop P");
        exit(EXIT_FAILURE);
    }
    return 1;
}

//hilfsmethode für getsem
void V(int semid, int semnum) {
    struct sembuf op = {
        semnum,
        1,
        0
    };
    if(semop(semid, &op, 1) < 0) {
        perror("semop V");
        exit(EXIT_FAILURE);
    }
}

void setupSems(int semid) {
    union semun args; 
    // Semaphore: max Prozesse, die mitlesen dürfen
    args.val = 1;
    if(semctl(semid, 0, SETVAL, args) < 0) {
        perror("semctl");
        exit(EXIT_FAILURE);
    } 

    // Semaphorne: freie bytes (256)
    args.val = MEMSZE;
    if(semctl(semid, 1, SETVAL, args) < 0) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    // Semaphore: lesbare bytes
    args.val = 0;
    if(semctl(semid, 2, SETVAL, args) < 0) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

void handleTimeout() {
    printf("Keine Nachricht für 3 Sekunden empfangen. Tschüss\n");
    timeout_triggered++;
    //HIER BLOß KEIN EXIT!
}