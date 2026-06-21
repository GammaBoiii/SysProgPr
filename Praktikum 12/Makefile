CFLAGS = -g -Wall -Wextra -pthread -fsanitize=address

all: server client

server: server.o
	gcc ${CFLAGS} -o server server.o
server.o: server.c commonfile.h
	gcc ${CFLAGS} -c server.c 
client: client.o
	gcc ${CFLAGS} -o client client.o
client.o: client.c commonfile.h
	gcc ${CFLAGS} -c client.c



.PHONY: all clean
clean:
	rm -f *.o *.dat server client