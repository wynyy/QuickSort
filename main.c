#include <stdio.h>
#include <stdlib.h>
#include "quickSort.h"

#define MaxValue 1000000

static int strToInt(char *);

/*
 * Global variable to store command line choice.
 */
int value=-1;

int main (int argc, char **argv) {
	/*
	 * Check the value argument.
	 */
	if (argc != 2) {
		printf("Error : one argument expected : length of array.\n");
		return 1;
	} else if ((value=strToInt(argv[1])) < 1) {
		printf("Error : negative value given.\n");
		return 1;
	} else if (value > MaxValue) {
		printf("Error : length of array too large.\n");
		return 1;
	}
	quickSort(value);
	return 0;
}

/*
 * Change string to is int value.
 */
static int strToInt(char *str) {
	int mem=0;
	for (int j=0; str[j]; j++) {
		if (str[j]>'9' || str[j]<'0') {
			printf("Error : number argument expected.\n");
			exit(1);
		} else {
			mem = mem*10+str[j]-'0';
		}
	}
	return mem;
}
