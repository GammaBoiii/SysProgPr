#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include "commonfile.h"

int sock = -1;
int canceled = 0;
FILE *src_file = NULL;
volatile sig_atomic_t interrupted = 0; //wäre mit atexit besser..

void handle_sigint(int sig) {
    (void) sig;
    interrupted = 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Aufruf: %s <Quelldatei> <Zieldatei>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *src_path = argv[1];
    char *dest_path = argv[2];

    // Prüfen, ob Quelldatei existiert und lesbar ist
    src_file = fopen(src_path, "rb"); // b wieder Absicherung, siehe man fopen
    if (!src_file)
    {
        perror("Fehler beim Öffnen der Quelldatei");
        return EXIT_FAILURE;
    }
    signal(SIGINT, handle_sigint);

    printf("Client startet mit ID %d\n", getpid());

   // int sock = -1;
    struct sockaddr_un address;
    Header header;
    char buffer[BUFFER_SIZE];

    // Socket erstellen
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket-Erstellung fehlgeschlagen");
        fclose(src_file);
        return EXIT_FAILURE;
    }

    
    if(interrupted) goto cleanup_error;

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX; //nur lokal
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1); //siehe man strncpy caveats

    // Verbindung zum server/socket aufbauen
    if (connect(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("Verbindung zum Server fehlgeschlagen. Läuft der Server?");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    if (interrupted) goto cleanup_error;
    // Client schickt pid zu Identifizierung:
    pid_t pid = getpid();
    pid_t* pidp = &pid;
    header.type = MSG_IDENT;
    header.size = sizeof(pid_t);
    if(send(sock, &header, sizeof(Header),MSG_NOSIGNAL ) < 0) { // header kündigt payload an (kein feste Buffergröße für send!!)
        perror("Senden fehlgeschlagen");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    if(send(sock, pidp, header.size,MSG_NOSIGNAL)< 0) { // payload
        perror("Senden fehlgeschlagen");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    if (interrupted) goto cleanup_error;
    // Als erstes Dateiname senden
    header.type = MSG_FILENAME;
    header.size = strlen(dest_path) + 1; // Inklusive Nullterminator \0
    if ((send(sock, &header, sizeof(Header), MSG_NOSIGNAL)) < 0)
    {
        perror("Senden fehlgeschlagen");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    if ((send(sock, dest_path, header.size, MSG_NOSIGNAL)) < 0)
    {
        perror("Senden fehlgeschlagen");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }

    // Antwort vom Server abwarten (OK nach Dateiname!!)
    if (read(sock, &header, sizeof(Header)) <= 0)
    {
        printf("Keine Antwort vom Server erhalten.\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }

    if (header.type == MSG_REJECT)
    {
        fprintf(stderr, "Zieldatei bereits auf Server!\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    else if (header.type == MSG_ERROR)
    {
        fprintf(stderr, "Fehler beim Erstellen der Datei!\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    else if (header.type != MSG_OK)
    {
        fprintf(stderr, "Ein allgemeiner Fehler ist aufgetreten.. Programm beendet sich!\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Server hat zugestimmt. Starte Übertragung...\n");

    // Daten auslesen und send
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0)
    {

        // Interrupt (strg + c)
        if (interrupted)
        {
            printf("\n[Client] Übertragung durch Benutzer abgebrochen.\n");
            canceled += 1;
            // Dem Server Bescheid geben, dass wir abbrechen!
            header.type = MSG_ERROR;
            header.size = 0;
            if (send(sock, &header, sizeof(Header), MSG_NOSIGNAL) == -1)
            {
                fclose(src_file);
                close(sock);
                perror("Fehler beim Senden des Errors");
                break;
            }
            break;
        }

        header.type = MSG_DATA;
        header.size = bytes_read;

        // Header senden
        if (send(sock, &header, sizeof(Header), MSG_NOSIGNAL) == -1)
        {
            fclose(src_file);
            close(sock);
            perror("Fehler beim Senden des Headers");
            break;
        }
        // Palyoad senden
        if (send(sock, buffer, bytes_read, MSG_NOSIGNAL) == -1)
        {
            fclose(src_file);
            close(sock);
            perror("Fehler beim Senden der Daten");
            break;
        }
    }

    // Prüfen, ob fread wegen eines Fehlers abgebrochen ist
    if (ferror(src_file))
    {
        perror("Fehler beim Lesen der Quelldatei");
        header.type = MSG_ERROR;
        header.size = 0;
        if(send(sock, &header, sizeof(Header), MSG_NOSIGNAL) <= 0) {
            fclose(src_file);
            close(sock);
            perror("Fehler beim Senden des ERROR");
            return EXIT_FAILURE;
        }
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }
    else
    {
        // 3. Phase: Fertig signalisieren
        header.type = MSG_DONE;
        header.size = 0;
        if(send(sock, &header, sizeof(Header), MSG_NOSIGNAL) <= 0) {
            fclose(src_file);
            close(sock);
            perror("Fehler beim Senden des DONE");
            return EXIT_FAILURE;
        }

        if (canceled == 0)
        {
            printf("Datei erfolgreich übertragen.\n");
        }
        else
        {
            printf("Dateiübertragung zwischendurch abgebrochen.!");
        }
    }

    fclose(src_file);
    close(sock);
    return EXIT_SUCCESS;

    cleanup_error:
    if(src_file) fclose(src_file);
    if(sock != -1) close(sock);
    return EXIT_FAILURE;
}