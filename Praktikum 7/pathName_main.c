#include "pathName.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    clock_t start = clock();
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
    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Die Suche hat %f Sekunden gedauert.\n", time_taken);
    exit(EXIT_SUCCESS);    
}