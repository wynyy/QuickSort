/*
 * Linearised QuickSort on a random int array.
 * To limits memory used why used a stack to store each step of the algorithm.
 */

#define QuickSort_C

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

/*
 * Creation of a LIFO (Last In First Out) (Stack).
 * Used to prevent stack overflow of recursive QuickSort.
 * For linear QuickSort FIFO used more memory.
 */
typedef struct {
	subA_t *lifo;
	short capacity,
		used;
} lifoSubA_t;

static lifoSubA_t initLifo(void);
static void extendsLifoCap(lifoSubA_t *);
static void addLifo(lifoSubA_t *, subA_t);
static subA_t pullLifo(lifoSubA_t *);
static void subQuickSort(lifoSubA_t *);

/*
 * Create a random int array of a the given length and sort it.
 */
void quickSort(int length) {
	int *array = randomArray(length);
	lifoSubA_t stack=initLifo();
	subA_t entireArray;
	entireArray.begin = array;
	entireArray.length = length;
	addLifo(&stack,entireArray);
	subQuickSort(&stack);
	if (sortCheck(array,length)) {
		printf("Array correctly sorted.\n");
	} else {
		printf("Array not sorted.\n");
	}
	free(array);
	free(stack.lifo);
}

/*
 * Initialize lifoSubA_t.
 */
static lifoSubA_t initLifo(void) {
	lifoSubA_t stack;
	if ((stack.lifo=malloc(sizeof(subA_t)*200))==NULL) {
		printf("Error : malloc() for lifoSubA_t init.\n");
		exit(1);
	}
	stack.used = 0;
	stack.capacity = 200;
	return stack;
}

/*
 * Extends capacity of the LIFO.
 * We only extends by 200 the capacity because we used the stack in a quick sort.
 * In other situation we should rather multiply by 2 to reduce complexity.
 */
static void extendsLifoCap(lifoSubA_t *stack) {
	if (stack->capacity > 1600) {
		/* 
		 * If we need a 16K stack, whe should have made an error previously (or a
		 * gigantic array is used).
		 */
		printf("Error : unable to extends capacity of stack, already a 16K.");
		exit(1);
	}
	subA_t *tmp;
	if ((tmp=realloc(stack->lifo,sizeof(subA_t)*(stack->capacity+200)))==NULL) {
		printf("Error : realloc() in extends stack capacity.\n");
		exit(1);
	}
	stack->lifo = tmp;
	stack->capacity += 200;
}

/*
 * Add value to stack.
 * The length must be greater than 1 to be added.
 */
static void addLifo(lifoSubA_t *stack, subA_t value) {
	if (stack->used == stack->capacity) {
		extendsLifoCap(stack);
	} else if (value.begin!=NULL && value.length>1) {
		stack->lifo[(stack->used)++] = value;
	}
}

/*
 * Pull the top of the stack or subA_t.begin = NULL if empty.
 */
static subA_t pullLifo(lifoSubA_t *stack) {
	if (!(stack->used)) {
		subA_t empty;
		empty.begin = NULL;
		return empty;
	} else {
		return stack->lifo[--(stack->used)];
	}
}

/*
 * Subdivision of the QuickSort.
 */
static void subQuickSort(lifoSubA_t *stack) {
	subA_t segmentSort=pullLifo(stack), tmp;
	int *array, i, j, pivot;
	while (segmentSort.begin != NULL) {
		array = segmentSort.begin;
		i = 1;
		j = segmentSort.length-1;
		pivot = array[0];
		while (i!=j) {
			while (i!=j && pivot>=array[i]) i++;
			while (i!=j && pivot<=array[j]) j--;
			if (i != j) {
				swap(array,i,j);
			}
		}
		if (array[j]<pivot) {
			swap(array,0,j);
			pivot = j;
		} else {
			swap(array,0,j-1);
			pivot=j-1;
		}
		/*
		 * Creation of the 2 subA_t obtened (if possible).
		 */
		j = segmentSort.length-pivot-1;
		j = (j>0)?(j):(0);
		i = pivot;
		i = (i>0)?(i):(0);
		segmentSort.begin = (i>1)?(array):(NULL);
		segmentSort.length = i;
		tmp.begin = (j>1)?(array+pivot+1):(NULL);
		tmp.length = j;
		/*
		 * At then end, the shortest one must be in segmentSort for the next while.
		 */
		if (i<j) {
			addLifo(stack,segmentSort);
			addLifo(stack,tmp);
		} else {
			addLifo(stack,tmp);
			addLifo(stack,segmentSort);
		}
		segmentSort = pullLifo(stack);
	}
}
