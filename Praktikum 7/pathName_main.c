#include <stdio.h>
#include <stdlib.h>
#include "pathName.h"

char* getFullPathName(const char *name, const char *ENVName);

int main(int argc, char const *argv[])
{
    // Argumente überprüfen
    // 1. keine Argumente:
    if (argc <= 2)
    {
        fprintf(stderr, "Fehlende Argumente für %s.\n(exit)\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else if (argc > 3)
    {
        fprintf(stdout, "Zu viele Argumente angegeben. Ignoriere die letzt %d Argumente.\n", argc - 3);
    }
    char *res;
    printf("Die suche ergibt: %s\n", (res = getFullPathName(argv[1], argv[2])) ? res : "kein Treffer...");
    free(res); // braucht man hier eigentlich nicht, weil exit das übernimmt.
    exit(EXIT_SUCCESS);    
}