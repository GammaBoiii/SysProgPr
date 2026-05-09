#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void doInts(int n);
void zeigeFeld(int *feld, int anzahl);
void sortieren(int *feld, int anzahl);
void parameter(char const *feld[], int anzhl);

int erg;
int n = 0;

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    // erg = scanf("%d", &n);
    // printf("params: %s \n", argv[1]);
    // n = atoi(argv[1]);
    //doInts(n);

    // printf("argv ist: %p\n", argv);
    // int *p1 = &argv[0];
    // printf("p1 ist: %p mit %d\n", p1, atoi(argv[0]));
    // parameter((int*)argv, argc);
    
    printf("===Test===\n");
    int x = 520;
    int *ptr = &x;
    printf("x: %d, ptr: %p, *ptr: %d\n", x, ptr, *ptr);
    printf("==========\n");

    parameter(argv, argc);

    return 0;
}


// Bestückt ein Array mit n Zufallszahlen.
void doInts(int n) {

    // Reserviert speicher für das dynamische Array, welches n ints aufnehmen kann.
    int *p = malloc(sizeof(int) * n);
    
    // Weist dem Array Zahlen zu und gibt die Adresse der jeweiligen Zahl aus.
    if(p) {
        for(int i = 0; i < n; i++) {
            p[i] = rand() % 10000;
            //printf("Zahl %d initialisiert mit: %d  -> %p\n",i+1, p[i], &p[i]);
        }
    }    
    
    zeigeFeld(p, n);
    free(p);
    
}

void zeigeFeld(int *feld, int anzahl) {
   // printf(sizeof(feld));
    sortieren(feld, anzahl);
    for(int i = 0; i < anzahl; i++) {
        //printf("Zahl %d: %d \n",i+1, feld[i]);
    }
}

// Sortiert die ein Array aus Pointern, die auf die Elemente des übergebenen Arrays zeigen, mit dem Bubble-Sort Algorithmus.
void sortieren(int *feld, int anzahl) {

    // Reserviert Speicher für ein Array von Zeigern, die auf die Elemente des übergebenen Arrays zeigen.
    int** ptrarray = malloc(sizeof(int*) * anzahl);

    // Weist jedem Zeiger die Adresse des entsprechenden Elements des übergebenen Arrays zu und gibt die Adresse und den Wert aus.
    for(int i = 0; i<anzahl; i++) {
        ptrarray[i] = &feld[i];
        printf("ptrarray[%d] = %p mit %d\n", i, ptrarray[i], *ptrarray[i]);
    }

    // Nun wird geschaut, welches der Adressen den größten Wert beinhaltet, und entsprechend sortiert.
    for(int i = 0; i < anzahl-1; i++) {
        int id_biggest = i;
        for(int j = i; j<anzahl-1; j++) {
            if(*ptrarray[id_biggest] < *ptrarray[j+1]) {
                //printf("New biggest: %d < %d\n", *ptrarray[id_biggest], *ptrarray[j+1]);
                id_biggest = j+1;
            }

            if(j == anzahl-2) {
                int *temp = ptrarray[i];
                ptrarray[i] = ptrarray[id_biggest];
                ptrarray[id_biggest] = temp;
            }
        }
    } 

    printf("\n========================\nSortierte Adressen:\n========================\n");
    for(int i = 0; i < anzahl; i++) {
        printf("%d: %p - %d\n", i, ptrarray[i], *ptrarray[i]);
    }


}

void parameter(char const *feld[], int anzhl)
{
    // Alles soll hier eigentlich als int gedrescht werden. Wehe es werden String oder sonstige nicht-INTs übergeben!

    char const **pntrarr = malloc(sizeof(char*) * (anzhl-1));

    for (int i = 0; i < anzhl-1; i++)
    {
        pntrarr[i] = feld[i+1];
        printf("Parameter %d: %s mit Adresse %p\n", i, pntrarr[i], pntrarr[i]);
    }

    printf("Das sind insgesamt %d Parameter, %d ohne den Programmnamen.\n", anzhl, anzhl-1);

    for(int i = 0; i < anzhl-1; i++) {
        int id_biggest = i;

        for(int j = i; j < anzhl-2; j++) {
            if(atoi(pntrarr[id_biggest]) < atoi(pntrarr[j+1])) {
                id_biggest = j+1;
            }
            
        }
        printf("Biggest: %s mit %d\n", pntrarr[id_biggest], *pntrarr[id_biggest]);
        char const *temp = pntrarr[i];
        pntrarr[i] = pntrarr[id_biggest];
        pntrarr[id_biggest] = temp;
    }
    printf("\n========================\nSortierte Adressen:\n========================\n");
    for(int i = 0; i < anzhl-1; i++) {
        //printf("%d: %p - %s\n", i, &feld[i], feld[i]);
       printf("%d: Pointer-Wert %p zeigt auf Zahl: %s\n", i + 1, (void*)pntrarr[i], pntrarr[i]);
    }
}

//gcc -Wall -fsanitize=address -g -o nums nints.c; ./nums 6




