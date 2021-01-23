default : a.out

clean :
	rm a.out *.o

a.out : main.o quickSort.o utils.o processQuickSort.o
	gcc quickSort.o utils.o main.o

main.o : main.c quickSort.h
	gcc -c main.c

quickSort.o : quickSort.c utils.h
	gcc -c quickSort.c

utils.o : utils.c utils.h
	gcc -c utils.c
processQuickSort.o : processQuickSort.c utils.h
	gcc -c processQuickSort.c -pthread
