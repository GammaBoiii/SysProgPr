#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void childStart(char* argv[]);

int main(int argc, char *argv[])
{
    // mindestens 2 Argumente ("./starter" und "Programmname") müssen übergeben werden
    if (argc < 2)
    {
        fprintf(stderr, "Aufrufparameter: %s <program> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // in Eltern und Kindprozess aufteilen:
    int npid = fork();
    if(npid == -1) {
        fprintf(stderr, "Fehler beim Erstellen des Kindprozesses aufgetreten.\n");
        exit(EXIT_FAILURE); 
    } else if(npid == 0) {
        // Kindprozess führt Programm aus:
        childStart(argv);
    } else {
        // Elternprozess
        //printf("Elternprozess (PID: %d) wartet auf Kindprozess (PID%d)\n", getpid(), npid);
        int status;
        while (1 == 1) {
            sleep(1);
            if(waitpid(npid, &status, WNOHANG) > 0) {
                printf("Kindprozess (PID: %d) mit Status %d beendet.\n", npid, WEXITSTATUS(status));
                exit(EXIT_SUCCESS);
            }
        }
        
    }
    return 0;
}

void childStart(char *argv[]) {
    printf("Test test %p und %p\n", argv[0], &argv[0]);
    // Programm als Kindprozess ausführen:
    
    int exc = execvp(argv[1], &argv[1]);
    
    // execvp() kehrt nur zurück, wenn ein Fehler auftritt. Daher können wir hier den Rückgabewert überprüfen.
    printf("exc: %d\n", exc);
}
