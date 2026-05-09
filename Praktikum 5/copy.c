#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "regular.h"

int main(int argc, char const *argv[])
{
    
    // if (argc != 3)
    // {
    //     fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
    //     return 1;
    // }

    char const *src = argv[1];
    char const *dest = argv[2];

    int fd_src;
    int fd_dest;
    ssize_t nread;
    char buffer[1024];

    if((fd_src = open(src, O_RDONLY)) == -1) {
        perror("Quelldatei existiert nicht!");
        return EXIT_FAILURE;
    }
    printf("fd_src: %d\n", fd_src);


    // Prüfen, ob Zieldatei bereits existiert
    // Wenn ein fd exisiter, dann gibts die Datei -> raus und Fehlermeldung
    if((fd_dest = open(dest, O_RDONLY)) > 0) {
        perror("Datei existiert bereits!\n");
        return EXIT_FAILURE;
    } else {
        // Ansonsten erstelle die Datei
        if((fd_dest = open(dest, O_WRONLY | O_CREAT, 0644)) == -1) {
            perror("Fehler beim Erstellen der Zieldatei!");
            return EXIT_FAILURE;
        }
    }
    printf("fd_dest: %d\n", fd_dest);

    // Zeiger Positionieren
    if(lseek(fd_src, 0, SEEK_SET) == -1) {
        perror("Fehler beim Positionieren des Zeigers in der Quelldatei!");
        return EXIT_FAILURE;
    }


    // Merke: read braucht eine Dateideskriptor, einen Pointer, bzw einen Speicherort (hier buffer) und eine Größe, die gebuffert werden kann (hier "buffer" - arraygröße)
    while((nread = read(fd_src, buffer, sizeof(buffer))) != 0) {
        if(nread == -1) {
            perror("Fehler beim kopieren!");
            return EXIT_FAILURE;
        }
        printf("\n\nnread: %zd\n\n", nread);
        buffer[nread] = '\0';
        write(fd_dest, buffer, nread);
        printf("SizeOf(buffer) = %ld vs nread %zd\n", sizeof(buffer), nread);
        printf("%s", buffer);
    }

    //printf("\n\n--------------------\n\n");

    //printf("Letzter Buffer: %s\n", buffer);

    return 0;
}
