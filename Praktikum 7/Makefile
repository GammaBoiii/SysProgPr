#Variablen:
CFLAGS = -g -Wall -Wextra -fsanitize=address

pathName: pathName_main.o pathName.o
	gcc ${CFLAGS} -o pathName pathName_main.o pathName.o

pathName_main.o: pathName_main.c pathName.h
	gcc ${CFLAGS} -c pathName_main.c

pathName.o: pathName.c pathName.h
	gcc ${CFLAGS} -c pathName.c

.PHONY: clean
clean:
	rm -f *.o pathName