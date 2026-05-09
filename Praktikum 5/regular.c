#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

int isRegularFile(const char *dateiname);

/*int main(int argc, char const *argv[])
{

    isRegularFile(argv[1]);
    return 0;
}*/

int isRegularFile(const char *dateiname) {
    int fd;
    struct stat statbuf;

    if((fd = open(dateiname, O_RDONLY)) == -1) {
        perror("Datei existiert nicht!");
        return EXIT_FAILURE;
    } else {
        // Datei existiert
        printf("Datei %s existiert!\n", dateiname);
        fstat(fd, &statbuf);

        //print result
        //printf("Datei: %s\n", dateiname);
        printf("Datei ist: %s (%d)\n", S_ISREG(statbuf.st_mode) ? "regulär" : "nicht regulär", statbuf.st_mode);
    }
}
