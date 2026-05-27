CFLAGS = -g -Wall -Wextra -fsanitize=address

all: server client

server: server.o
	gcc $(CFLAGS) -o server server.o

client: client.o
	gcc $(CFLAGS) -o client client.o

server.o: server.c
	gcc $(CFLAGS) -c server.c

client.o: client.c
	gcc $(CFLAGS) -c client.c

.PHONY: clean
clean:
	rm -f *.o server client