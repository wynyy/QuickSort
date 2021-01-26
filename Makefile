default : a.out

clear :
	rm a.out *.o

a.out : main.o quickSort.o utils.o processQuickSort.o
	gcc quickSort.o utils.o main.o processQuickSort.o -pthread

main.o : main.c quickSort.h
	gcc -c main.c -O3

quickSort.o : quickSort.c utils.h
	gcc -c quickSort.c -O3

utils.o : utils.c utils.h
	gcc -c utils.c -O3

processQuickSort.o : processQuickSort.c processQuickSort.h utils.h
	gcc -c processQuickSort.c -O3
