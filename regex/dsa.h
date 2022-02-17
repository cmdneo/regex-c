#ifndef RE_DSA_H_INTERNAL
#define RE_DSA_H_INTERNAL

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

// #define GENERIC_STACK_INIT(T)                                                 \
// 	typedef struct CONCAT_TOKENS(T, _stack_T) {                           \
// 		int size;                                                     \
// 		int cap;                                                      \
// 		T *data;                                                      \
// 	} CONCAT_TOKENS(T, _stack_T);                                         \
// 	static CONCAT_TOKENS(T, _stack_T) * CONCAT_TOKENS(T, _stack_create)() \
// 	{                                                                     \
// 		CONCAT_TOKENS(T, _stack_T) *ret = ALLOC(ret);                 \
// 		if (ret == NULL)                                              \
// 			return NULL;                                          \
// 		T *data = N_ALLOC(data, 1);                                   \
// 		if (data == NULL) {                                           \
// 			FREE(ret);                                            \
// 			return NULL;                                          \
// 		}                                                             \
// 		ret->data = data;                                             \
// 		ret->cap = 1;                                                 \
// 		return ret;                                                   \
// 	}                                                                     \
// 	static int CONCAT_TOKENS(T, _stack_resize)(                           \
// 		CONCAT_TOKENS(T, _stack_T) * stk, int new_cap)                \
// 	{                                                                     \
// 		assert(stk);                                                  \
// 		assert(new_cap > 0);                                          \
// 		T *data = N_REALLOC(stk->data, new_cap);                      \
// 		if (data == NULL)                                             \
// 			return -1;                                            \
// 		stk->data = data;                                             \
// 		stk->cap = new_cap;                                           \
// 		return 0;                                                     \
// 	}                                                                     \
// 	static void CONCAT_TOKENS(T, _stack_destroy)(                         \
// 		CONCAT_TOKENS(T, _stack_T) * *stk)                            \
// 	{                                                                     \
// 		assert(stk);                                                  \
// 		assert(*stk);                                                 \
// 		FREE((*stk)->data);                                           \
// 		FREE(*stk);                                                   \
// 		*stk = NULL;                                                  \
// 	}                                                                     \
// 	static int CONCAT_TOKENS(T, _stack_push)(                             \
// 		CONCAT_TOKENS(T, _stack_T) * stk, int n)                      \
// 	{                                                                     \
// 		assert(stk);                                                  \
// 		if (stk->size == stk->cap) {                                  \
// 			if (CONCAT_TOKENS(T, _stack_resize)(                  \
// 				    stk, stk->cap * 2) != 0)                  \
// 				return -1;                                    \
// 		}                                                             \
// 		stk->data[stk->size++] = n;                                   \
// 		return 0;                                                     \
// 	}                                                                     \
// 	static T CONCAT_TOKENS(T,                                             \
// 			       _stack_pop)(CONCAT_TOKENS(T, _stack_T) * stk)  \
// 	{                                                                     \
// 		assert(stk);                                                  \
// 		assert(stk->size > 0);                                        \
// 		return stk->data[--stk->size];                                \
// 	}                                                                     \
// 	enum { CONCAT_TOKENS(T, _STACK_ITEM_SIZE) = sizeof(T) }

#endif
