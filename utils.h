#ifndef Utils_H
#define Utils_H

#ifndef Utils_C
#define Utils_P extern
#else
#define Utils_P
#endif

typedef struct {
	int *begin,		/* Begin of the sub-array. */
		length;		/* Length of the sub-array. */
} subA_t;

Utils_P int *randomArray(int);
Utils_P int sortCheck(int *, int);
Utils_P void swap(int *, int, int);

#endif
