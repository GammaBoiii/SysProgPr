#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include "commonfile.h"

int main(int argc, char const *argv[])
{
    if(argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Key erstellen
    key_t key;
    if((key = ftok("server", 42)) < 0) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    printf("Key (Server) : %d\n", key); 

    // Ringbuffer/Memory Erstellen
    int shmid;
    if((shmid = shmget(key, sizeof(RingBuffer), 0600)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    // ... einblenden/attachen
    RingBuffer* shm;
    if((shm=shmat(shmid, NULL, 0)) == (RingBuffer *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    
}