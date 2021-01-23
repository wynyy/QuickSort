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
static void freeFilo(lifoProc_t *);
static void extendsLifoCap(lifoProc_t *);
static void addFilo(lifoProc_t *, subA_t);
static subA_t pullFilo(lifoProc_t *, char);
static subA_t subANextLoop(lifoProc_t *, subA_t, subA_t);
static int createChild(lifoProc_t *, char);

/*
 * Create a random int array of a the given length and sort it using process.
 * The number of process create is also given.
 */
void processQuickSort(int length, int process) {
	lifoProc_t *stack=initLifo();
	int *array=randomArrayShared(length);
	printf("Process number : %d.\n",process);
	subA_t entireArray;
	entireArray.begin = array;
	entireArray.length = length;
	stack->filo[0] = entireArray;
	if (!createChild(stack,process)) {
		return;
	}
	if (sortCheck(array,length)) {
		printf("Array correctly sorted.\n");
	} else {
		printf("Array not sorted.\n");
	}
	freeFilo(stack);
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
		stack->used = 1;	/* Init at 1 because next we write entireArray. */
		stack->workingProcess = 0;
	}
}

/*
 * Free space of a lifoProc_t.
 * Also destroy mutex and semaphore.
 */
static void freeFilo(lifoProc_t *stack) {
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
 */
static void addFilo(lifoProc_t *stack, subA_t value) {
	pthread_mutex_lock(&stack->mutex);
	if (stack->used == stack->capacity) {
		extendsLifoCap(stack);
	} else {
		if (!(stack->used)) {
			sem_post(&stack->pauses);
		}
		stack->filo[(stack->used)++] = value;
	}
	pthread_mutex_unlock(&stack->mutex);
}

/*
 * Pull the top of the stack or subA_t.begin = NULL if empty.
 */
static subA_t pullFilo(lifoProc_t *stack, char alreadyWaiting) {
	printf("Pull %d\n",getpid());
	pthread_mutex_lock(&stack->mutex);
	subA_t top;
	if (!alreadyWaiting) {
		/*
		 * The process was working.
		 */
		if (stack->used) {
			top = stack->filo[--(stack->used)];
		} else if (stack->workingProcess > 1) {
			--(stack->workingProcess);
			pthread_mutex_unlock(&stack->mutex);
			sem_wait(&stack->pauses);
			return pullFilo(stack,1);
		} else {
			/*
			 * Stack empty and no process working.
			 */
			sem_post(&stack->pauses);
			top.begin = NULL;
		}
	} else {
		/*
		 * Process was waiting.
		 * Other process make sem_post().
		 * At leat one working process.
		 */
		if (stack->used) {
			top = stack->filo[--(stack->used)];
			(stack->workingProcess)++;
		} else if (stack->workingProcess) {
			/*
			 * Process faster get the value.
			 */
			pthread_mutex_unlock(&stack->mutex);
			sem_wait(&stack->pauses);
			return pullFilo(stack,1);
		} else {
			/*
			 * No process working.
			 */
			sem_post(&stack->pauses);
			top.begin = NULL;
		}
	}
	pthread_mutex_unlock(&stack->mutex);
	return top;
}

/*
 * Determine the next subArray for a quick sort loop.
 * If left and right NULL, give the stack value.
 * If both not NULL, add the largest one to the stack.
 */
static subA_t subANextLoop(lifoProc_t *stack, subA_t left, subA_t right) {
	if (left.begin == right.begin) {
		return pullFilo(stack,0);
	} else if (left.begin == NULL) {
		return right;
	} else if (right.begin == NULL) {
		return left;
	} else if (right.length < left.length) {
		addFilo(stack,left);
	} else {
		addFilo(stack,right);
	}
}

/*
 * Subdivision of the QuickSort.
 */
static void subQuickSort(lifoProc_t *stack) {
	subA_t segmentSort=pullFilo(stack,0), tmp;
	int *array, i, j, pivot;
	while (segmentSort.begin != NULL) {
		printf("boucle %d\n",getpid());
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
		 * Creation of the 2 subA_t obtened (if possible else begin = NULL).
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
		 * Or NULL if sort done.
		 */
		segmentSort = subANextLoop(stack,segmentSort,tmp);
	}
}

/*
 * Create all child.
 */
static int createChild(lifoProc_t *stack, char childNumber) {
	int i=0, id=1;
	for (i=1; i<childNumber && id; i++) {
		if ((id=fork())==-1) {
			printf("Warning : only %d process working.\n",i);
			childNumber = i;
		} else if (!id) {
			printf("Child : %d\n",getpid());
			/*
			 * Child process.
			 */
			subQuickSort(stack);
		}
	}
	if (id) {
		/*
		 * Main process.
		 */
		printf("Father process.\n");
		subQuickSort(stack);
		while (--childNumber) {
			wait(NULL);
		}
	}
	return id;
}
