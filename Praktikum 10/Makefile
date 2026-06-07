CFLAGS = -g -Wall -Wextra -fsanitize=address

all: verbraucher erzeuger ringbuffer

verbraucher: verbraucher.o
	gcc $(CFLAGS) -o verbraucher verbraucher.o

erzeuger: erzeuger.o
	gcc $(CFLAGS) -o erzeuger erzeuger.o

verbraucher.o: verbraucher.c commonfile.h
	gcc $(CFLAGS) -c verbraucher.c

erzeuger.o: erzeuger.c commonfile.h
	gcc $(CFLAGS) -c erzeuger.c


ringbuffer: ringbuffer.o commonfile.h
	gcc $(CFLAGS) -o ringbuffer ringbuffer.o

ringbuffer.o: ringbuffer.c commonfile.h
	gcc $(CFLAGS) -c ringbuffer.c

.PHONY: clean
clean:
	rm -f *.o verbraucher erzeuger ringbuffer