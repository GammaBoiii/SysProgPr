CFLAGS = -g -Wall -Wextra -fsanitize=address

all: ringbuffer

ringbuffer: ringbuffer.o
	gcc $(CFLAGS) -o ringbuffer ringbuffer.o

ringbuffer.o: ringbuffer.c commonfile.h
	gcc $(CFLAGS) -c ringbuffer.c

.PHONY: all clean
clean:
	rm -f *.o ringbuffer target.dat