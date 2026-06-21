#pragma once

#define SOCKET_PATH "/tmp/mysocket"
#define BUFFER_SIZE 1024

typedef enum {
    MSG_FILENAME, // Client sendet Zieldateiname
    MSG_OK,       // Server bestätigt (bereit)
    MSG_REJECT,   // Server lehnt ab (Datei existiert bereits)
    MSG_DATA,     // Client sendet Datenblock
    MSG_DONE,     // Client signalisiert Übertragungsende
    MSG_ERROR     // Fehler signalisieren
} MsgType;

typedef struct {
    MsgType type;
    int size;     // Größe der nachfolgenden Daten (z.B. String-Länge oder Blockgröße)
} Header;