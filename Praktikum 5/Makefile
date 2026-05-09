prog: copy.o regular.o
	gcc -o prog copy.o regular.o

copy.o: copy.c regular.h
	gcc -c copy.c

regular.o: regular.c regular.h
	gcc -c regular.c