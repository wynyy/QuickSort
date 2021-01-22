/*
 * Evoluton of quickSort.c with multi-processing.
 *
 * The stack is independant, have built-in semaphore and mutex (mutex are faster when
 * there is no concurrency).
 */

#define ProcessQuickSort_C

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "utils.h"

/*
 * Create a random int array of a the given length and sort it using process.
 * The number of process create is also given.
 */
void quickSort(int length, int process) {
	printf("Process number : %d.\n",process);
}
