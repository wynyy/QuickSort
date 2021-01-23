#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quickSort.h"
#include "processQuickSort.h"

#define MaxValue 1000000

static int strToInt(char *);
static void sizeCheck(char *);
static void processCheck(int, char **);
static void parseArg(int, char **);
/*
 * Global variable to store command line choice.
 */
int size=-1;
char process=-1;

int main (int argc, char **argv) {
	parseArg(argc,argv);
	if (size==-1) {
		printf("Error : size of array expected.\n");
		exit(1);
	} else if (process == -1) {
		quickSort(size);
	} else {
		processQuickSort(size,process);
	}
	return 0;
}

/*
 * Change string to is int value.
 */
static int strToInt(char *str) {
	int mem=0;
	for (int j=0; str[j]; j++) {
		if (str[j]>'9' || str[j]<'0') {
			if (str[j] == '-') {
				printf("Error : positif number expected.\n");
				exit(1);
			}
			printf("Error : number expected.\n");
			exit(1);
		} else {
			mem = mem*10+str[j]-'0';
		}
	}
	return mem;
}

/*
 * Check the size given in parameter.
 */
static void sizeCheck(char *str) {
	if (size != -1) {
		printf("Error : size of array already chosen.\n");
		exit(1);
	} else if ((size=strToInt(str))<1 || size>MaxValue) {
		printf("Error : length of array too large (limits : %d).\n",MaxValue);
		exit(1);
	}
}

/*
 * Check the number of process given.
 */
static void processCheck(int argc, char **argv) {
	if (process != -1) {
		printf("Error : number of process already chose.\n");
		exit(1);
	} else if (argv[0][2]) {
		printf("Error : did you mean '-p' ?\n");
		exit(1);
	} else if (argc == 1) {
		printf("Error : number expected after '-p'.\n");
		exit(1);
	} else if ((process=strToInt(argv[1]))<-1 || process>MaxProcess) {
		printf("Error : number of process too large (limits : %d).\n",MaxProcess);
		exit(1);
	}
}

/*
 * Analyse argv of command line.
 */
static void parseArg(int argc, char **argv) {
	for (int i=1; i<argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'p') {
				processCheck(argc-i,&argv[i]);
				i++;
			} else {
				printf("Error : command '%s' unknow.\n",argv[i]);
				exit(1);
			}
		} else {
			sizeCheck(argv[i]);
		}
	}
}
