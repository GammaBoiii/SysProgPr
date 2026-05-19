// Aufgabe 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

void forking();
void doChild();
void doParent(int npid);

int main(int argc, char const *argv[])
{
    /* code */
    forking();
    return 0;
}

void forking()
{
    int npid;
    // npid - Kindprozessid, die das Elternteil sieht (next PID)
    // ppid - Parent rpozess id, die kindprozess sieht
    // pid - aktuelle prozessid

    // kindprozess ereugen
    npid = fork();

    if (npid == -1)
    {
        fprintf(stderr, "Fehler bei fork. Code: %d, Fehler: %s\n", errno, strerror(errno));
    }
    else if (npid == 0)
    {
        // Kindprozess

        doChild();
    }
    else
    {
        // Elternprozess

        doParent(npid);
    }
}

void doChild()
{
    printf("Ich bin ein Kindprozess mit meiner PID: %d und meiner Parent PID: %d\n", getpid(), getppid());

    // bis 1.000.000.000 zählen
    for(long i=0; i<5000000000; i++)
    {
        // zählt nur sinnlos..
    }

    //sich beenden
    exit(0);
}
void doParent(int npid)
{
    int status;
    printf("Ich bin ein Elternprozess mit meiner PID: %d und meiner Kind PID: %d\n", getpid(), npid);
    /*wait(&status);
    printf("Kinderexitstatus: pid: %d mit status %d\n", npid, status);
    exit(status);
    */
   for(;;) {
    sleep(1);
     printf("Elternprozess mit PID: %d wartet auf Kindprozess mit PID: %d\n", getpid(), npid);
     if (waitpid(npid, &status, WNOHANG) > 0) {
         printf("Kinderexitstatus: pid: %d mit status %d\n", npid, status);
         exit(status);
     }
   }
}
