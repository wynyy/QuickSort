/*
 * Evoluton of quickSort.c -> multi-process.
 *
 * The stack is independant, have built-in semaphore and mutex (mutex are faster when
 * there is no concurrency).
 */

#define ProcessQuickSort_C
#define _GNU_SOURCE 	/* Needed to used mremap(). */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "utils.h"
#include "processQuickSort.h"

/*
 * Creation of a LIFO (Last In First Out) (Stack).
 * Used to prevent stack overflow of recursive QuickSort.
 * For linear QuickSort FIFO used more memory.
 *
 * Built-in mutex and semaphore.
 * When there is no working process and stack empty, return NULL subA_t to everything.
 */
typedef struct {
	subA_t *filo;			/* Stack of subA_t. */
	int capacity,			/* Capacity of the stack. */
		used,				/* Place of the stack used. */
		workingProcess;		/* Number of working process. */
	pthread_mutex_t mutex;	/* Access mutex to push or pull value (when not empty). */
	sem_t pauses;			/* If empty, the semaphore value is 0 and process whose
								trying to pull are waiting the semaphore.
								Not block mutex for push. */
} lifoProc_t;

static lifoProc_t *initLifo(void);
static void freeLifo(lifoProc_t *);
static void extendsLifoCap(lifoProc_t *);
static void addLifo(lifoProc_t *, subA_t);
static subA_t pullLifo(lifoProc_t *);

/*
 * Create a random int array of a the given length and sort it using process.
 * The number of process create is also given.
 */
void processQuickSort(int length, int process) {
	lifoProc_t *stack=initLifo();
	int *array=randomArrayShared(length), childP[MaxProcess];
	printf("Process number : %d.\n",process);
	subA_t entireArray;
	entireArray.begin = array;
	entireArray.length = length;
	addLifo(stack,entireArray);
	if (sortCheck(array,length)) {
		printf("Array correctly sorted.\n");
	} else {
		printf("Array not sorted.\n");
	}
	freeLifo(stack);
	munmap(array,4*length);
}

/*
 * Init the lifoProc_t.
 * - Allocation in shared space memory.
 * - Allocation int shared space memory of the filo.
 * - Init semaphore.
 * - Init mutex and other value.
 */
static lifoProc_t *initLifo(void) {
	lifoProc_t *stack;
	if ((stack=mmap(NULL,sizeof(lifoProc_t),PROT_WRITE|PROT_READ,MAP_ANON
			|MAP_SHARED,-1,0))==MAP_FAILED) {
		printf("Error : mmap() for lifoProc_t init.\n");
		exit(1);
	} else if ((stack->filo=mmap(NULL,4*200,PROT_READ|PROT_WRITE
			,MAP_ANON|MAP_SHARED,-1,0))==MAP_FAILED) {
		printf("Error : mmap for lifoProc_t->lifo init.\n");
		exit(1);
	} else if (sem_init(&stack->pauses,1,0)==-1) {
		printf("Error : sem_init() lifoProc_t init.\n");
		exit(1);
	} else {
		pthread_mutexattr_t attrmutex;
		pthread_mutexattr_init(&attrmutex);
		pthread_mutexattr_setpshared(&attrmutex,PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&stack->mutex,&attrmutex);
		stack->capacity = 200;
		stack->used = 0;
		stack->workingProcess = 0;
	}
}

/*
 * Free space of a lifoProc_t.
 * Also destroy mutex and semaphore.
 */
static void freeLifo(lifoProc_t *stack) {
	sem_destroy(&stack->pauses);
	pthread_mutex_destroy(&stack->mutex);
	munmap(stack->filo,4*stack->capacity);
	munmap(stack,sizeof(stack));
}

/*
 * Extends capacity of the LIFO.
 * We only extends by 200 the capacity because we used the stack in a quick sort.
 * In other situation we should rather multiply by 2 to reduce complexity.
 */
static void extendsLifoCap(lifoProc_t *stack) {
	if (stack->capacity > 16000) {
		/* 
		 * If we need a 16K stack, whe should have made an error previously (or a
		 * gigantic array is used).
		 */
		printf("Error : unable to extends capacity of stack, already a 16K.");
		exit(1);
	}
	subA_t *tmp;
	if ((tmp=mremap(stack->filo,sizeof(subA_t)*stack->capacity,
			sizeof(subA_t)*(stack->capacity+200),MREMAP_MAYMOVE,NULL))==MAP_FAILED) {
		printf("Error : mremap in extends stack capacity.\n");
	exit(1);
	}
	stack->filo = tmp;
	stack->capacity += 200;
}

/*
 * Add value to stack.
 * The length must be greater than 1 to be added.
 * Concurrency ignore.
 */
static void addLifo(lifoProc_t *stack, subA_t value) {
	if (stack->used == stack->capacity) {
		extendsLifoCap(stack);
	}
	stack->filo[(stack->used)++] = value;
}

/*
 * Pull the top of the stack or subA_t.begin = NULL if empty.
 * Concurrency ignore.
 */
static subA_t pullLifo(lifoProc_t *stack) {
	if (!(stack->used)) {
		subA_t empty;
		empty.begin = NULL;
		return empty;
	} else {
		return stack->filo[--(stack->used)];
	}
}
