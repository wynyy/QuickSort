#define Utils_C

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Create a random int array with a given length.
 */
int *randomArray(int length) {
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
