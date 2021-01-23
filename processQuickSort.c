/*
 * Evoluton of quickSort.c -> multi-process.
 *
 * The stack is independant, have built-in semaphore and mutex (mutex are faster when
 * there is no concurrency).
 */

#define ProcessQuickSort_C

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
/*
 * Create a random int array of a the given length and sort it using process.
 * The number of process create is also given.
 */
void processQuickSort(int length, int process) {
	lifoProc_t *stack=initLifo();
	int *array=randomArrayShared(length), childP[MaxProcess];
	printf("Process number : %d.\n",process);
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
