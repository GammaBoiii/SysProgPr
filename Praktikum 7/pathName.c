#include <stdio.h>
#include <stdlib.h>
#include <string.h> //fuer strlen
#include <dirent.h> //fuer Verzeichnisarbit
#include <errno.h>  //fuer Fehlerbehandlung

char *getFullPathName(const char *name, const char *ENVName)
{
    DIR* myDir;

    // ENV auslesen:
    char *envalue = getenv(ENVName);

    // Existiert ENV?:
    if (envalue == NULL)
    {
        fprintf(stderr, "Die Umgebungsvariable \"%s\" existiert nicht.\n", ENVName);
        return NULL;
    }

    // ENV einmal ausgeben:
    printf("\nHier ist der komplette ENV print von %s: \n%s\n\n", ENVName, envalue);

    // ENV aufteilen
    // Buffer für einzelne Strings/Pfade:
    char *input_copy = strdup(envalue);    // envvalue einmal kopieren, damit das original mit ":" erhalten bleibt. Siehe man strdup (muss free() werden)
    char *token = strtok(input_copy, ":"); // siehe man strtok
    int i = 1;                             // Zähler. Optional, sieht aber schön aus.
    while (token != NULL)
    {
        printf("Suche in: %s\n", token);

        // Jeder Token ist nun ein Verzeichnis, was nach dem gesuchten Filename durchsucht werden muss:
        myDir = opendir(token);
        if (myDir == NULL)
        { // Im Fehlerfall (kein gültiges Verzeichnis) wird einfach diese iteration übersprungen.
            fprintf(stderr, "Etwas ist schiefgelaufen. Code: %d, Fehler: %s (continue)\n\n", errno, strerror(errno));
            i++;
            token = strtok(NULL, ":"); // Nächsten token aktivieren.
            continue;
        }
        
        struct dirent *entry;
        while ((entry = readdir(myDir)) != NULL)
        {
            // Gucken, ob sich die gesuchte Datei hier drinne aufhält:
            if (strcmp(name, entry->d_name) == 0)
            {
                // Erfolgsnachricht:
                printf("\t\t|->%s wurde in diesem Verzeichnis gefunden!\n", name);
                printf("\t\t|->Der ganze Pfad lautet: %s/%s\n\n", token, entry->d_name);

                // Gesamten pfad bauen:
                char *buffer = malloc(sizeof(char) * (strlen(token) + strlen(entry->d_name) + 2));
                sprintf(buffer, "%s/%s", token, entry->d_name);

                // Erfolgreiche Suche, methode wird zum beenden vorbereitet:
                free(input_copy);
                closedir(myDir);
                return buffer;
            }
        }
        printf("\n");
        token = strtok(NULL, ":");
        i++;
        closedir(myDir);
    }
    printf("Die Datei \"%s\" konnte nicht im ENV \"%s\" gefunden werden.\n", name, ENVName);

    // Hier auch noch kurz temporäre Ressourcen entladen:
    free(input_copy);
    
    return NULL;
}

/* Achtung alter komischer Code: */
/*
if(bufferS == NULL) {
        fprintf(stderr, "Fehler bei der Speicherallokation.\n(exit)\n");
        return NULL;
    } else {
        int i = 0; // Zählt einzelne char Stellen
        int j = 0; // Zählt in einem Pfad die Stellen
        while(envalue[i] != '\0') {
            if(envalue[i+1] == '\0') {
                // Ende des ENV erreicht, BufferS ausgeben
                bufferS[j] = '\0';
                printf("\t%d\t", i);
                printf("\tGefunden: %s\n", bufferS);
                free(bufferS);
                break;
            }
            if(envalue[i] != filter) {
                bufferS[j] = envalue[i];
                i++;
                j++;
            }
            else
            {
                // Filter gefunden:
                // BufferS neu aufsetzen
                bufferS[j] = '\0';
                printf("\t%d\t", i);
                printf("\tGefunden: %s\n", bufferS);
                free(bufferS);
                bufferS = malloc(strlen(envalue) * sizeof(char));
                j = 0;
                i++;
            }
        }
    }
*/
