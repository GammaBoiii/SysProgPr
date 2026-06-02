#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "commonfile.h"

int main(int argc, char const *argv[])
{
    // Argumente checken
    if(argc != 3) {
		fprintf(stderr, "Usage: %s \'erzeuger\' <msg>\n", argv[0]);
        exit(EXIT_FAILURE);
	}

    // Verbindung zum Server herstellen
    // Key erstellen
    key_t key;
    if((key = ftok("server", 42)) < 0) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    printf("Key (Client): %d\n", key);

    // aus Datei Nachricht lesen
    int fd;
    if((fd = open(argv[2], O_RDONLY)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Ringbuffer/Memory Erstellen
    int shmid;
    if((shmid = shmget(key, sizeof(RingBuffer), IPC_CREAT | 0600)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    // ... einblenden/attachen
    RingBuffer* shm;
    if((shm=shmat(shmid, NULL, 0)) == (RingBuffer *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Nachricht senden
    char buffer[1024];
    ssize_t bytesRead;
    while((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        printf("Nachricht: %s\n", buffer);
    }

    close(fd);
    return 0;
}
