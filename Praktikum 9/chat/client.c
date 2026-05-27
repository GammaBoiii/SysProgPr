#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

struct message {
    long type;
    char text[105];
};

int main(int argc, char const *argv[])
{
    struct message send_msg;

    // Arguemente prüfen:
    if(argc != 3) {
        fprintf(stderr, "Falsche Parameter angegeben. Nutzung: %s [Quelldatei] [Zieldatei]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Prüfen ob Quelldatei existiert:
    int fd;
    if((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("Fehler beim Öffnen der Quelldatei");
        exit(EXIT_FAILURE);
    }

    // Kontaktaufnahme mit Server:
    key_t key;
    if((key = ftok("/home/johann/Documents/mysource", 42)) == -1) {
        fprintf(stderr, "Fehler beim Erstellen des Tokens");
        exit(EXIT_FAILURE);
    }
    printf("Message Key erstellet %d\n", key);  
    int msgid = msgget(key, 0666);
    if(msgid == -1) {
        fprintf(stderr, "Fehler beim Zugriff auf die Message Queue!\n-->Server muss zuerst gestartet werden!<--\n");
        exit(EXIT_FAILURE);
    }
    
    printf("message key: %d queue id: %d\n", key, msgid);

    //Nachricht schreiben
    send_msg.type = 1;
    char *buffer = malloc(sizeof(char) * (strlen(argv[1]) + strlen(argv[2]) + 2));
    if(buffer == NULL) {
        fprintf(stderr, "Fehler bei malloc. Code: %d, Fehler: %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, "%s:%s", argv[1], argv[2]);

    //printf("Baue Buffer Stinrg: %s\n", buffer);
    //printf("Größe vom buffer: %lu\n", strlen(buffer));

    strcpy(send_msg.text, buffer);

    printf("Sende Nachricht: %s\n", send_msg.text);
    msgsnd(msgid, &send_msg, sizeof(send_msg.text), 0);

    printf("Nachricht gesendet: %s\n", send_msg.text);

    free(buffer);

    return 0;
}
