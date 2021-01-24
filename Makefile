default : a.out

clear :
	rm a.out *.o

a.out : main.o quickSort.o utils.o processQuickSort.o
	gcc quickSort.o utils.o main.o processQuickSort.o -pthread -g -Wall -Wextra -pedantic -fno-omit-frame-pointer

main.o : main.c quickSort.h
	gcc -c main.c -g -Wall -Wextra -pedantic -fno-omit-frame-pointer

quickSort.o : quickSort.c utils.h
	gcc -c quickSort.c -g -Wall -Wextra -pedantic -fno-omit-frame-pointer

utils.o : utils.c utils.h
	gcc -c utils.c -g -Wall -Wextra -pedantic -fno-omit-frame-pointer

processQuickSort.o : processQuickSort.c processQuickSort.h utils.h
	gcc -c processQuickSort.c -pthread -g -Wall -Wextra -pedantic -fno-omit-frame-pointer
