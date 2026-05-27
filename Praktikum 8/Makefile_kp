#Variablen:
CFLAGS = -g -Wall -Wextra -fsanitize=address

prog: kp.o
	gcc ${CFLAGS} -o prog kp.o

kp.o: kp.c
	gcc ${CFLAGS} -c kp.c

.PHONY: clean
clean:
	rm -f *.o prog