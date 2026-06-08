#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>   //ftok
#include <sys/types.h> //ftok
#include <sys/shm.h>   //shared memory
#include <sys/sem.h>   //semaphoren
#include <unistd.h>    //fork
#include <signal.h>    //signal handling
#include <sys/wait.h>  //wait (für child)
#include "commonfile.h"

volatile sig_atomic_t timeout_triggered = 0; //diese variable ist signal (asynchron) sicher
int timeouts = 0;

struct Cleaner myCleaner;

void handleTimeout(int signum) {
    (void)signum; //warnung bei unused parameter umgehen..
   // printf("Keine Nachricht für 3 Sekunden empfangen. Tschüss\n");
    timeout_triggered = 1;
    //HIER BLOß KEIN EXIT!
}

int doParent(int shmid, int semid, const char *path); // erzeuger
void doChild(int shmid, int semid);                    // verbraucher

// hilfsmethode für putsem
void P(int semid, int semnum) {
    struct sembuf op = {
        semnum,
        -1,
        0 // SEM_UNDO autom. Freigeben der Semaphoren (man semop) -> schlgt bei großen binärdateien fehl!!!
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
                timeout_triggered = 0;
                if(timeouts < 2) {
                    timeouts++;
                    printf("\tWarte...\t");
                    fflush(stdout);
                    alarm(1);                    
                    continue;
                } else {
                    printf("\n");
                    return 0; //timer ausgelöst
                }
        
            } else {
                continue; //kein timer signal, also erstmal weiter warten
            }
        }
        perror("semop P safe");
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
    // Semaphore: max Prozesse, die erzeugen dürfen
    // DEPRICATED
    /*args.val = 1;
    if(semctl(semid, 0, SETVAL, args) < 0) {
        perror("semctl 0");
        exit(EXIT_FAILURE);
    }*/ 

    // Semaphorne: freie bytes (256)
    args.val = MEMSZE;
    if(semctl(semid, 0, SETVAL, args) < 0) {
        perror("semctl 1");
        exit(EXIT_FAILURE);
    }

    // Semaphore: lesbare bytes
    args.val = 0;
    if(semctl(semid, 1, SETVAL, args) < 0) {
        perror("semctl 2");
        exit(EXIT_FAILURE);
    }

    // Semaphore: max Prozesse, die konsumieren dürfen
    // DEPRICATED
    /*args.val = 1;
    if(semctl(semid, 3, SETVAL, args) < 0) {
        perror("semctl 3");
        exit(EXIT_FAILURE);
    } */
}


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

// DEPRECATED
void cleanExitChild(int signum) {
    printf("Signal %d empfangen, beende Kindprozess...\n", signum);
    timeout_triggered = 1; //lässt Kind denken, dass Timer getriggert ist
}

int main(int argc, char const *argv[])
{
    // Argumente checken
    if (argc != 2)
    {
        fprintf(stderr, "Anwendung: %s <QUelldatei>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Key erstellen
    key_t key;
    if ((key = ftok(argv[0], 42)) < 0)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // Ringbuffer/Memory Erstellen
    int shmid;
    if ((shmid = shmget(key, sizeof(RingBuffer), IPC_CREAT | 0600)) < 0)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    myCleaner.shmid = shmid;
    // ... einblenden/attachen
    RingBuffer *init_shm;
    if ((init_shm = shmat(shmid, NULL, 0)) == (RingBuffer *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    init_shm->in_pos = 0;
    init_shm->out_pos = 0;
    if (shmdt(init_shm) < 0)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    
    // Semaphorne erstellen
    int semid;
    if ((semid = semget(key, 2, IPC_CREAT | IPC_EXCL | 0600)) < 0)
    {
        printf("Semaphore existiert bereits!\n");
        perror("semget");
        exit(EXIT_FAILURE);
    }
    myCleaner.semid = semid;
    // Semaphoren initiieren
    setupSems(semid);

    signal(SIGINT, cleanExit);  // signalhandler für sauberes aufräumen bei strg+c
    signal(SIGTERM, cleanExit); // signalhandler für sauberes aufräumen bei kill

    int npid;
    if ((npid = fork()) < 0)
    {
        perror("fork");
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if (npid == 0)
    {
        // Übernimmt verbraucher prozess
        doChild(shmid, semid);
    }
    else
    {

        signal(SIGINT, cleanExit);  // signalhandler für sauberes aufräumen bei strg+c
        signal(SIGTERM, cleanExit); // signalhandler für sauberes aufräumen bei kill

        // Übernimmt erzeugerprozess
        if((doParent(shmid, semid, argv[1]) == EXIT_FAILURE)) {
            // wenn doParent fehlschlägt, soll auch der kindprozess nicht weiterlaufen, da sonst evtl. unkontrolliert auf die semaphoren/shmem zugegriffen wird, die ja vielleicht schon gelöscht wurden
            kill(npid, SIGTERM); //kindprozess mit signal beenden
            wait(NULL); //auf kind warten
            cleanup(); //aufräumen
            exit(EXIT_FAILURE);
        }

        wait(NULL); // auf kind warten

        // Aufräumarbeiten
        /*
        shmctl(shmid, IPC_RMID, NULL); //shared memory löschen
        semctl(semid, 0, IPC_RMID); //semaphoren löschen
        */
        cleanup();
    }
    printf("\n");
    return 0;
}

int doParent(int shmid, int semid, const char *path)
{
    RingBuffer *rb;
    if ((rb = shmat(shmid, NULL, 0)) == (RingBuffer *)-1)
    {
        perror("shmat");
        return(EXIT_FAILURE);
    }

    FILE *source;
    if ((source = fopen(path, "rb")) == NULL)
    { // das b ist laut manual eine Absicherung..
        perror("fopen");
        return(EXIT_FAILURE);
    }

    int cch;           // current char der gelesend wird
    int bytecount = 0; // zum tracken der bytes, um die '*' auszugeben

    while ((cch = fgetc(source)) != EOF)
    {
        P(semid, 0); // nach schreibplatz im buffer schaue
        rb->buffer[rb->in_pos] = cch;
        rb->in_pos = (rb->in_pos + 1) % MEMSZE; // ringbuffer, daher modulo
        V(semid, 1);                            // kündigt neues byte an

        bytecount++;
        if (bytecount % 100 == 0)
        {
            printf("*");
            fflush(stdout);
        }
    }

    fclose(source);
    shmdt(rb);
    return EXIT_SUCCESS;
}

void doChild(int shmid, int semid)
{
    RingBuffer *rb;
    if ((rb = shmat(shmid, NULL, 0)) == (RingBuffer *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    FILE *target;
    if ((target = fopen(TFILE, "wb")) == NULL)
    { // auch hier das b als absicherung nach manual
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    signal(SIGALRM, handleTimeout);
    //signal(SIGINT, cleanExitChild);
    //signal(SIGTERM, cleanExitChild);
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    int bytecount = 0;

    while (1) // muss hier für "immer" warten, da ja nicht die dateigröße kennt
    { 
        alarm(1);

        if (P_safe(semid, 1) == 0)
        {
            break;
        }
        timeouts = 0;
        alarm(0);

        char cch = rb->buffer[rb->out_pos];
        rb->out_pos = (rb->out_pos + 1) % MEMSZE;

        V(semid, 0);
        fputc(cch, target);

        bytecount++;
        if (bytecount % 100 == 0)
        {
            printf("#");
            fflush(stdout);
        }
    }
    fclose(target);
    shmdt(rb);
    printf("Kind stirbt jetzt...\n");
    exit(EXIT_SUCCESS);
}
