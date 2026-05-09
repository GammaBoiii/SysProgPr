#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char in;

    do
    {
        printf("Geben Sie ein Zeichen ein: ");
        scanf("%c", &in);
        fflush(stdin);
        printf("Das eingegebene Zeichen ist: \n %c \n %d \n 0x%x \n", in, in, in); // Hex?
    } while (in != 'q'); 
    return 0;
}
