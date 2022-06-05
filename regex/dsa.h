#ifndef REGEX_DSA_H_INTERNAL
#define REGEX_DSA_H_INTERNAL

typedef struct pairs_T {
	int cap;
	int size;
	int *pairs;
} pairs_T;

pairs_T *pairs_create();
void pairs_destroy(pairs_T **skv);
int pairs_insert(pairs_T *skv, int v1, int v2);
int pairs_search(pairs_T *vec, int v1);

/**
 * @brief Integer stack
 */
typedef struct stack_T {
	int size;
	int cap;
	int *data;
} stack_T;

stack_T *stack_create();
void stack_destroy(stack_T **stk);
int stack_push(stack_T *stk, int n);
int stack_pop(stack_T *stk);

#endif
