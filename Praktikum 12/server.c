#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include "commonfile.h"

int server_fd = -1;

// Cleanup-Funktion beim Beenden
void cleanup() {
    if (server_fd != -1) {
        close(server_fd);
    }
    unlink(SOCKET_PATH);
    printf("\n[Server] Socket entfernt und sauber beendet.\n");
}

// Signalhandler für Strg+C
void handle_sigint(int sig) {
    (void) sig;
    exit(EXIT_SUCCESS); // Löst atexit(cleanup) aus
}

// Die Thread-Funktion für jeden Client
void* client_handler(void* arg) {
    int sock = *(int*)arg; //das ist jetzt auf dem thread stack (privat)
    free(arg); // Dynamischen Speicher für die Socket-ID sofort freigeben

    // Thread in den detached Zustand versetzen, um Ressourcenlecks zu vermeiden
    pthread_detach(pthread_self());

    Header header;
    char buffer[BUFFER_SIZE];
    pid_t pid;
    FILE* dest_file = NULL;

    // Client id senden (zum Identifizieren in der Konsole)
    if(read(sock, &header, sizeof(Header)) <= 0 || header.type != MSG_IDENT) {
        printf("[Thread] Protokollfehler beim Empfang der Client ID.\n");
        close(sock);
        return NULL;
    }
    if(read(sock, &pid, header.size) <= 0) {
        printf("Lesen fehlgeschlagen");
        close(sock);
        return NULL;
    }

    // hier sollte als erstes Dateiname ankommen
    if (read(sock, &header, sizeof(Header)) <= 0 || header.type != MSG_FILENAME) {
        printf("[Thread] Protokollfehler beim Empfang des Dateinamens.\n");
        close(sock);
        return NULL;
    }

    memset(buffer, 0, sizeof(buffer));
    if((read(sock, buffer, header.size)) <= 0) {
        printf("Lesen fehlgeschlagen");
        close(sock);
        return NULL;
    }
    printf("[Thread] Client %d möchte Datei schreiben: %s\n",pid, buffer);

    // Prüfen, ob die Datei bereits existiert
    struct stat st;
    if (stat(buffer, &st) == 0) {
        printf("[Thread] Ablehnung: Datei '%s' existiert bereits.\n", buffer);
        header.type = MSG_REJECT;
        header.size = 0;
        send(sock, &header, sizeof(Header), 0);
        close(sock);
        return NULL;
    }

    // Datei zum Schreiben öffnen
    dest_file = fopen(buffer, "wb");
    if (!dest_file) { //fopen returned nur NULL, nicht -1!
        printf("[Thread] Fehler: Datei konnte nicht erstellt werden.\n");
        header.type = MSG_ERROR;
        send(sock, &header, sizeof(Header), 0);
        close(sock);
        return NULL;
    }

    // Bestätigung an Client senden
    header.type = MSG_OK;
    if((send(sock, &header, sizeof(Header), 0)) < 0) {
        printf("[Thread] Fehler: Message konnte nicht zugestellt werden.\n");
        header.type = MSG_ERROR;
        // send(sock, &header, sizeof(header),0); //das hier ist sinnlos
        close(sock);
        return NULL;
    }

    // Daten empfangen
    while (1) {
        if (read(sock, &header, sizeof(Header)) <= 0) {
            printf("[Thread] Verbindungsabbruch durch den Client.\n");
            break;
        }

        if (header.type == MSG_DONE) {
            printf("[Thread] Übertragung erfolgreich abgeschlossen.\n");
            break;
        }

        if (header.type == MSG_ERROR) {
            printf("[Thread] Client hat die Übertragung wegen eines Fehlers abgebrochen.\n");
            break;
        }

        if (header.type == MSG_DATA) {
            // Datenblock lesen
            int bytes_received = 0;
            while (bytes_received < header.size) {
                int n = read(sock, buffer + bytes_received, header.size - bytes_received);
                if (n <= 0) break; //gibt nix mehr zum schreiben
                bytes_received += n;
            }

            // In Datei schreiben
            if (fwrite(buffer, 1, header.size, dest_file) != (size_t) header.size) {
                perror("[Thread] Fehler beim Schreiben in die Zieldatei");
                header.type = MSG_ERROR;
                send(sock, &header, sizeof(Header), 0);
                break;
            }
        }
    }

    if (dest_file) fclose(dest_file);
    close(sock);
    return NULL;
}

int main() {
    struct sockaddr_un address;

    // exit und cleanup aufsetzen.
    atexit(cleanup);
    signal(SIGINT, handle_sigint);

    // Socket erstellenn
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket erstellen fehlgeschlagen");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(struct sockaddr_un)); // 0 setzen "={0}"
    address.sun_family = AF_UNIX; // kein internt
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1); //pfad /tmp/mysocket + extra zeiche für ggf \0 (siehe man strncpy caveats)

    // löscht datei (laut man), also vorher Socket clearne
    unlink(SOCKET_PATH);

    // Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == -1) {
        perror("Bind fehlgeschlagen");
        exit(EXIT_FAILURE);
    }

    // Listen, mit 5 initialisiert laut Vorlesung, F. 223
    if (listen(server_fd, 5) == -1) {
        perror("Listen fehlgeschlagen");
        exit(EXIT_FAILURE);
    }

    printf("[Server] Paralleler Server gestartet. Warte auf Verbindungen...\n");

    // permaschleife
    while (1) {
        int new_socket = accept(server_fd, NULL, NULL);
        if (new_socket == -1) {
            perror("Accept fehlgeschlagen");
            continue;
        }

        // Speicher allokieren damit der nächste client einen sauberen, eigenen "Platz" bekommt.. (THREADS HABEN KEINEN EIGENEN SPEICHERBEREICH!!)
        int* client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("Malloc fehlgeschlagen");
            close(new_socket);
            continue;
        }
        *client_sock = new_socket;

        pthread_t thread_id;
        // Thread starten und Socket-Zeiger übergeben
        if (pthread_create(&thread_id, NULL, client_handler, (void*)client_sock) != 0) {
            perror("Thread-Erstellung fehlgeschlagen");
            close(new_socket);
            free(client_sock);
        }
    }

    return 0;
}