
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void signalHandlerP(int signum)
{
    //printf("Signal-Parent %d empfangen\n", signum);
    // exit(signum);
}

void signalHandlerC(int signum)
{
    //printf("Signal-Child %d empfangen\n", signum);
    // exit(signum);
}

void doChild()
{
    int cnt = 0;
    while (cnt < 5)
    {
        //printf("kindprozess wartet auf signal\n");
        
        pause();
        printf("Kindprozess PID-Nr %d\n", getpid());
        cnt++;
        
      //  printf("kindprozess sendet signal\n");
        
        kill(getppid(), SIGUSR2);
        
    }
}

void doParent(int npid)
{
    sleep(1);
    //printf("Elternprozess sendet erstes signal\n");
    printf("Elternprozess PID-Nr %d\n", getpid());
    kill(npid, SIGUSR2);
    
    int cnt = 0;
    while (cnt < 4)
    {
        //printf("elternprozess wartet auf signal\n");
        
        pause();
        printf("Elternprozess PID-Nr %d\n", getpid());
        cnt++;
        
        //printf("elternprozess sendet signal\n");
        
        kill(npid, SIGUSR2);
        
    }
}

int main()
{
    int npid = fork();
    if (npid == -1)
    {
        fprintf(stderr, "Fehler beim Erstellen des Kindprozesses\n");
        exit(EXIT_FAILURE);
    }
    else if (npid == 0)
    {
        // Kindprozess
        signal(SIGUSR2, signalHandlerC);
        doChild();
    }
    else
    {
        // Elternprozess
        signal(SIGUSR2, signalHandlerP);
        doParent(npid);
    }

    return 0;
}
