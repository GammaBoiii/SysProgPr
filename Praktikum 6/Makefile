prog: statinfo.o checkDir.o
	gcc -o prog checkDir.o statinfo.o

checkDir.o: checkDir.c statinfo.h
	gcc -c checkDir.c

statinfo.o: statinfo.c statinfo.h
	gcc -c statinfo.c

