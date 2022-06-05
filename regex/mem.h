#ifndef RE_MEM_H_INTERNAL
#define RE_MEM_H_INTERNAL

#include <stdlib.h> /* malloc, free, realloc */

#define ALLOC(ptr) calloc(1, sizeof(*ptr))
#define REALLOC(ptr) realloc((ptr), sizeof(*ptr))

#define N_ALLOC(ptr, n) calloc((n), sizeof(*ptr))
#define N_REALLOC(ptr, n) realloc((ptr), (n) * sizeof(*ptr))

#define FREE(ptr) free(ptr)

#endif
