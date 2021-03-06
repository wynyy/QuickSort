#define Utils_C

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

/*
 * Create a random int array with a given length.
 */
int *randomArrayPrivate(int length) {
	int *array;
	if ((array=malloc(4*length))==NULL) {
		printf("Error : malloc() for array init.\n");
		exit(1);
	}
	srand(time(NULL));
	for (int i=0; i<length; i++) {
		array[i] = rand();
	}
	return array;
}


/*
 * Create a random int array with a given length.
 * In shared memory space.
 */
int *randomArrayShared(int length) {
	int *array;
	if ((array=mmap(NULL,4*length,PROT_READ|PROT_WRITE,MAP_ANON
			|MAP_SHARED,-1,0))==MAP_FAILED) {
		printf("Error : malloc() for array init.\n");
		exit(1);
	}
	srand(time(NULL));
	for (int i=0; i<length; i++) {
		array[i] = rand();
	}
	return array;
}

/*
 * Check if an array is sort in ascending order.
 */
int sortCheck(int *array, int length) {
	for (int i=1; i<length; i++) {
		if (array[i]<array[i-1]) {
			return 0;
		}
	}
	return 1;
}

/*
 * Swap value.
 */
void swap(int *array, int i, int j) {
	int tmp=array[i];
	array[i] = array[j];
	array[j] = tmp;
}
