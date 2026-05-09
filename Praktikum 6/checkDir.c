#include "statinfo.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h> // fuer opendir
#include <dirent.h> // fuer opendir
#include <string.h> // fuer strlen

int checkDir(const char* dir);

int main(int argc, char const *argv[])
{
    if(argc < 2) {
        perror("Fehlender Parameter");
        exit(EXIT_FAILURE);
    } else {
        checkDir(argv[1]);
    }
    return 0;
}

int checkDir(const char* dir) {
    int fd;
    struct stat buf;
    

    //prüfe, ob fd ein verzeichnis ist:
    if((fd = open(dir, O_RDONLY)) == -1) {
        perror("Verzeichnis existiert nicht\n");
    } else {
        fstat(fd, &buf);
        if(S_ISDIR(buf.st_mode)) {
            printf("\n\nDie eingegebene Datei ist ein Verzeichnis\n");
            printf("---------------------------------------------\n");
            DIR *mydir = opendir(dir);
            struct dirent *entry;
            while((entry = readdir(mydir)) != NULL) {
                if(entry->d_name[0] == '.' || (entry->d_name[0] == '.' && entry->d_name[1] == '.')) {
                    continue;
                }
                printf("Datei: %s wird geprüft:\n", entry->d_name);
                char* buffer = malloc(sizeof(char) * (strlen(dir) + strlen(entry->d_name) + 2));
                sprintf(buffer, "%s/%s", dir, entry->d_name);
                checkStatusInfo(buffer);
                free(buffer);
            }
            closedir(mydir);
        } else if(S_ISREG(buf.st_mode)) {
            printf("\n\nDie eingegebene Datei ist eine reguläre Datei\n");
            printf("---------------------------------------------\n");
            checkStatusInfo(dir);
        } else {
            printf("Die eingegebene Datei ist weder ein Verzeichnis noch eine reguläre Datei\n");
            return EXIT_FAILURE;
        }
    }
}
