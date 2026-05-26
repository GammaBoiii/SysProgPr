CFLAGS = -g -Wall -Wextra -fsanitize=address

starter: starter.o
	gcc $(CFLAGS) -o starter starter.o

starter.o: starter.c
	gcc $(CFLAGS) -c starter.c

.PHONY: clean
clean:
	rm -f *.o starter