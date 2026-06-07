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

int timeout_triggered = 0;
int timeouts = 0;
typedef struct {
    char buffer[MEMSZE];
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
} myCleaner;

void cleanup() {
    printf("Aufräumarbeiten werden durchgeführt...\n");
    if(shmctl(myCleaner.shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl IPC_RMID");
    }
    if(semctl(myCleaner.semid, 0, IPC_RMID) < 0) {
        perror("semctl IPC_RMID");
    }
}


void cleanExit(int signum) {
    printf("Zeit zum Aufräumen:\n");
    printf("Signal %d empfangen, beende...\n", signum);
    cleanup();
    printf("Es wurde aufgeräumt, adios!\n");
    exit(EXIT_SUCCESS);
}

void cleanExitChild(int signum) {
    printf("Signal %d empfangen, beende Kindprozess...\n", signum);
    timeout_triggered = 1; //lässt Kind denken, dass Timer getriggert ist
}

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
        SEM_UNDO //autom. Freigeben der Semaphoren (man semop)
     };
    if(semop(semid, &op, 1) < 0) {
        perror("semop P");
        exit(EXIT_FAILURE);
    }
}

//hilfsmethode für putsem, aber mit absicherung vor anderen unterbrechungen/signalen
//erstaz für P() bei dem 3Sekunden Timer
int P_safe(int semid, int semnum) {
    struct sembuf op = { semnum, -1, SEM_UNDO };
    
    while (semop(semid, &op, 1) < 0) { //whileschleife startet semop hier neu, wenn es unterbrochen wurde (gibt dann nämlich -1 aus)
        if (errno == EINTR) {
            if(timeout_triggered) {
                if(timeouts < 2) {
                    timeouts++;
                    alarm(1);
                    timeout_triggered = 0; //timer zurücksetzen, damit es nicht direkt wieder getriggert wird, wenn es schon getrig
                    continue;;
                } else {
                    printf("\n");
                    return 0; //timer ausgelöst
                }
        
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
        SEM_UNDO
    };
    if(semop(semid, &op, 1) < 0) {
        perror("semop V");
        exit(EXIT_FAILURE);
    }
}

void setupSems(int semid) {
    union semun args; 
    // Semaphore: max Prozesse, die erzeugen dürfen
    args.val = 1;
    if(semctl(semid, 0, SETVAL, args) < 0) {
        perror("semctl 0");
        exit(EXIT_FAILURE);
    } 

    // Semaphorne: freie bytes (256)
    args.val = MEMSZE;
    if(semctl(semid, 1, SETVAL, args) < 0) {
        perror("semctl 1");
        exit(EXIT_FAILURE);
    }

    // Semaphore: lesbare bytes
    args.val = 0;
    if(semctl(semid, 2, SETVAL, args) < 0) {
        perror("semctl 2");
        exit(EXIT_FAILURE);
    }

    // Semaphore: max Prozesse, die konsumieren dürfen
    args.val = 1;
    if(semctl(semid, 3, SETVAL, args) < 0) {
        perror("semctl 3");
        exit(EXIT_FAILURE);
    } 
}

void handleTimeout() {
   // printf("Keine Nachricht für 3 Sekunden empfangen. Tschüss\n");
    printf("\tWarte...\t");
    fflush(stdout);
    timeout_triggered++;
    //HIER BLOß KEIN EXIT!
}