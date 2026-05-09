#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
    struct test {
        char c;
        int x;
        float z;
    } test1;
    test1.c = 'z';
    test1.x = 1000000;
    test1.z = 1000000;

    struct test *ptr = &test1;
    printf("struct liegt an der Adresse %p\n", ptr);
    char *ptrc = &test1.c;
    int *ptrx = &test1.x;
    float *ptrz = &test1.z;
    printf("die variablen liegen bei: \n%p \n%p \n%p\n", ptrc, ptrx, ptrz);
    printf("Die größen sind: \n%lu \n%lu \n%lu\n", sizeof(test1.c), sizeof(test1.x), sizeof(test1.z));

    union struct_test {
        struct test test1;
        unsigned char chars[sizeof(struct test)];
    } utest;

    utest.test1.c = 'z';
    utest.test1.x = 1000000;
    utest.test1.z = 1000000;

    for(int i = 0; i<sizeof(struct test); i++) {
        printf("%02x ", utest.chars[i]);
    }

    return 0;
}
