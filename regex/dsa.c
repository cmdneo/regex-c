#include <limits.h> /* INT_MIN */
#include <assert.h>

#include "dsa.h"
#include "mem.h"

pairs_T *pairs_create()
{
	pairs_T *ret = ALLOC(ret);
	if (ret == NULL)
		return NULL;
	int *pairs = N_ALLOC(pairs, 2);
	if (pairs == NULL) {
		free(ret);
		return NULL;
	}

	*ret = (pairs_T){ .cap = 1, .pairs = pairs };

	return ret;
}

void pairs_destroy(pairs_T **prs)
{
	assert(prs);
	assert(*prs);

	FREE((*prs)->pairs);
	FREE(*prs);
	*prs = NULL;
}

static int pairs_resize(pairs_T *prs, int new_cap)
{
	assert(prs);
	assert(new_cap > 0);

	int *pairs = N_REALLOC(prs->pairs, 2 * new_cap);
	if (pairs == NULL)
		return -1;

	prs->pairs = pairs;
	prs->cap = new_cap;

	return 0;
}

int pairs_insert(pairs_T *prs, int v1, int v2)
{
	assert(prs);

	if (prs->cap == prs->size) {
		size_t new_cap = prs->cap * 2;
		if (pairs_resize(prs, new_cap) != 0)
			return -1;
	}

	prs->pairs[2 * prs->size] = v1;
	prs->pairs[2 * prs->size + 1] = v2;

	prs->size++;

	return 0;
}

int pairs_search(pairs_T *prs, int v1)
{
	for (int i = 0; i < prs->size; i++)
		if (prs->pairs[2 * i] == v1)
			return prs->pairs[2 * i + 1];

	assert(!"Key not found");
	return 0;
}

stack_T *stack_create()
{
	stack_T *ret = ALLOC(ret);
	if (ret == NULL)
		return NULL;
	int *data = N_ALLOC(data, 1);
	if (data == NULL) {
		FREE(ret);
		return NULL;
	}

	*ret = (stack_T){ .data = data, .cap = 1 };
	return ret;
}

static int stack_resize(stack_T *stk, int new_cap)
{
	assert(stk);
	assert(new_cap > 0);

	int *data = N_REALLOC(stk->data, new_cap);
	if (data == NULL)
		return -1;

	stk->data = data;
	stk->cap = new_cap;

	return 0;
}

void stack_destroy(stack_T **stk)
{
	assert(stk);
	assert(*stk);

	FREE((*stk)->data);
	FREE(*stk);
	*stk = NULL;
}

/**
 * @brief 
 * 
 * @param stk 
 * @param n 
 * @return int 0 on success, -1 otherwise
 */
int stack_push(stack_T *stk, int n)
{
	assert(stk);

	if (stk->size == stk->cap) {
		if (stack_resize(stk, stk->cap * 2) != 0)
			return -1;
	}

	stk->data[stk->size++] = n;

	return 0;
}

int stack_pop(stack_T *stk)
{
	assert(stk);
	assert(stk->size > 0);

	return stk->data[--stk->size];
}
