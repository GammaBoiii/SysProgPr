#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "commonfile.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Aufruf: %s <Quelldatei> <Zieldatei>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* src_path = argv[1];
    char* dest_path = argv[2];

    // Prüfen, ob Quelldatei existiert und lesbar ist
    FILE* src_file = fopen(src_path, "rb");
    if (!src_file) {
        perror("Fehler beim Öffnen der Quelldatei");
        return EXIT_FAILURE;
    }

    int sock = -1;
    struct sockaddr_un address;
    Header header;
    char buffer[BUFFER_SIZE];

    // Socket erstellen
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket-Erstellung fehlgeschlagen");
        fclose(src_file);
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    // Verbindung zum Server aufbauen
    if (connect(sock, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == -1) {
        perror("Verbindung zum Server fehlgeschlagen. Läuft der Server?");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }

    sleep(4);
    // 1. Phase: Zieldateiname senden
    header.type = MSG_FILENAME;
    header.size = strlen(dest_path) + 1; // Inklusive Nullterminator
    send(sock, &header, sizeof(Header), 0);
    send(sock, dest_path, header.size, 0);

    // Antwort vom Server abwarten
    if (read(sock, &header, sizeof(Header)) <= 0) {
        printf("Keine Antwort vom Server erhalten.\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }

    if (header.type == MSG_REJECT) {
        fprintf(stderr, "Server lehnt Übertragung ab: Zieldatei existiert bereits beim Server!\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    } else if (header.type == MSG_ERROR) {
        fprintf(stderr, "Server meldet internen Fehler beim Erstellen der Datei.\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    } else if (header.type != MSG_OK) {
        fprintf(stderr, "Unerwartetes Protokoll-Feedback vom Server.\n");
        fclose(src_file);
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Server hat zugestimmt. Starte Übertragung...\n");

    // 2. Phase: Daten lesen und senden
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        header.type = MSG_DATA;
        header.size = bytes_read;
        
        // Header senden
        if (send(sock, &header, sizeof(Header), 0) == -1) {
            perror("Fehler beim Senden des Headers");
            break;
        }
        // Nutzdaten senden
        if (send(sock, buffer, bytes_read, 0) == -1) {
            perror("Fehler beim Senden der Daten");
            break;
        }
    }

    // Prüfen, ob fread wegen eines Fehlers abgebrochen ist
    if (ferror(src_file)) {
        perror("Fehler beim Lesen der Quelldatei");
        header.type = MSG_ERROR;
        header.size = 0;
        send(sock, &header, sizeof(Header), 0);
    } else {
        // 3. Phase: Fertig signalisieren
        header.type = MSG_DONE;
        header.size = 0;
        send(sock, &header, sizeof(Header), 0);
        printf("Datei erfolgreich übertragen.\n");
    }

    fclose(src_file);
    close(sock);
    return EXIT_SUCCESS;
}