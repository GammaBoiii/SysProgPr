#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void cleanup() {
    // Message que löschen:
    int key = ftok("/home/johann/Documents/mysource", 42);
    if(key == -1) { 
        fprintf(stderr, "Fehler beim Erstellen des Tokens");
        exit(EXIT_FAILURE);
    }
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if(msgid == -1) {
        fprintf(stderr, "Fehler beim Zugriff auf die Message Queue");
        exit(EXIT_FAILURE);
    }
    if(msgctl(msgid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Fehler beim Löschen der Message Queue");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

struct message {
    long type;
    char text[100];
};

int main()
{
    struct message rec_msg;

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGKILL, cleanup);
    // Message que erstellen:
    int key = ftok("/home/johann/Documents/mysource", 42);
    if(key == -1) { 
        fprintf(stderr, "Fehler beim Erstellen des Tokens");
        exit(EXIT_FAILURE);
    }
    printf("Server myTok: %d\n", key);
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if(msgid == -1) {
        fprintf(stderr, "Fehler beim Erstellen der Message Queue");
        exit(EXIT_FAILURE);
    }
    printf("message key: %d queue id: %d\n", key, msgid);
    for(;;) {
        printf("Server läuft und erwartet Nachricht (%ld)\n", time(NULL));
        msgrcv(msgid, &rec_msg, sizeof(rec_msg.text), 1, IPC_NOWAIT);
        if(rec_msg.text[0] != '\0') {
            printf("\nNachricht empfangen:\t%s\n", rec_msg.text);
            //Handling der Nachricht:
            char* token = strtok(rec_msg.text, ":");
            char* token2 = strtok(NULL, ":");
            printf("\t->Quelldatei:\t%s\n\t->Zieldatei:\t%s\n", token, token2);
            rec_msg.text[0] = '\0'; // Buffer leeren
            printf("\n");
        }
        sleep(1);        
    }
    return 0;
}
