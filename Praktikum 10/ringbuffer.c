#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> //ftok
#include <sys/types.h> //ftok
#include <sys/shm.h> //shared memory
#include <sys/sem.h> //semaphoren
#include <unistd.h> //fork
#include <signal.h> //signal handling
#include <sys/wait.h> //wait (für child)
#include "commonfile.h"


void doParent(int shmid, int semid, const char* path); // erzeuger
void doChild(int shmid, int semid); // verbraucher

int main(int argc, char const *argv[])
{
    // Argumente checken
    if(argc != 2) {
        fprintf(stderr, "Anwendung: %s <QUelldatei>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Key erstellen
    key_t key;
    if((key = ftok(argv[0], 42)) < 0) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Ringbuffer/Memory Erstellen
    int shmid;
    if((shmid = shmget(key, sizeof(RingBuffer), IPC_CREAT | 0600)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    // ... einblenden/attachen
    RingBuffer* init_shm;
    if((init_shm=shmat(shmid, NULL, 0)) == (RingBuffer *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    init_shm->in_pos=0;
    init_shm->out_pos=0;
    if(shmdt(init_shm) < 0) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // Semaphorne erstellen
    int semid;
    if((semid = semget(key, 3, IPC_CREAT | 0600)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    // Semaphoren initiieren
    setupSems(semid);

    int npid;
    if((npid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if(npid == 0) {
        //Übernimmt verbraucher prozess
        doChild(shmid, semid);
    } else {
        // Übernimmt erzeugerprozess
        doParent(shmid, semid, argv[1]);

        wait(NULL); //auf kind warten

        shmctl(shmid, IPC_RMID, NULL); //shared memory löschen
        semctl(semid, 0, IPC_RMID); //semaphoren löschen
    }
    return 0;
}

void doParent(int shmid, int semid, const char* path) {
    RingBuffer* rb;
    if((rb=shmat(shmid, NULL, 0)) == (RingBuffer *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    FILE* source;
    if((source = fopen(path, "rb")) == NULL) { // das b ist laut manual eine Absicherung..
        perror("fopen");
        exit(EXIT_FAILURE);
    } 

    int cch; //current char der gelesend wird
    int bytecount = 0; //zum tracken der bytes, um die '*' auszugeben

    while((cch = fgetc(source)) != EOF) {
        P(semid, 0);
        V(semid,0); //verhindert deadlock, wenn buffer voll ist, sonst blockiert P(semid,1) ewig, weil verbraucher nicht in sem0 reinkommt, wenn es erst am Ende befreit werden würde (daher das anklopfen
        
        P(semid,1); //nach schreibplatz im buffer schaue
        rb->buffer[rb->in_pos] = cch;
        rb->in_pos=(rb->in_pos + 1) % MEMSZE; //ringbuffer, daher modulo
        V(semid,2); //kündigt neues byte an

        bytecount++;
        if(bytecount % 100 == 0) {
            printf("*");
            fflush(stdout);
        }
    }

    fclose(source);
    shmdt(rb);
    exit(EXIT_SUCCESS);
    
}

void doChild(int shmid, int semid) {
    RingBuffer* rb;
    if((rb=shmat(shmid, NULL, 0)) == (RingBuffer * ) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    FILE* target; 
    if((target = fopen(TFILE, "wb")) == NULL) {// auch hier das b als absicherung nach manual
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    signal(SIGALRM, handleTimeout);
    int bytecount = 0;


    while(1) { //muss hier für "immer" warten, da ja nicht die dateigröße kennt
        alarm(3);

        if(P_safe(semid, 2) == 0) {
            break;
        }
        alarm(0);

        char cch = rb->buffer[rb->out_pos];
        rb->out_pos=(rb->out_pos + 1) % MEMSZE;
    
        V(semid,1);
        fputc(cch, target);

        bytecount++;
        if(bytecount % 100 == 0) {
            printf("#");
            fflush(stdout);
        }
        
    }
    fclose(target);
    shmdt(rb);
    exit(EXIT_SUCCESS);
    
}


