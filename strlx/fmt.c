#include <stdlib.h>

#include "strlx/strlx.h"
#include "strlx/fmt.h"

static size_t fmt_hash(str s)
{
	size_t h = 0;

	for (isize i = 0; i < s.size; i++)
		h += s.data[i] * FMT_HMAP_HASH_MULT;

	return h % FMT_HMAP_NBUCKETS;
}

static fmt_ll *fmt_hmap_lookup(str tname)
{
	size_t h = fmt_hash(tname);

	for (fmt_ll *f = fmt_tnf[h]; f->next != NULL; f = f->next) {
		if (str_cmp(tname, f->tname) == 0)
			return f;
	}

	return NULL;
}

static fmt_ll *fmt_hmap_insert(fmt_ll *entry)
{
	if (fmt_hmap_lookup(entry->tname) != NULL)
		return NULL;
	fmt_ll *bk_last = fmt_tnf[fmt_hash(entry->tname)];
	for (; bk_last->next != NULL; bk_last = bk_last->next)
		;

	bk_last->next = entry;

	return entry;
}

int fmt_register(str tname, printer print_fn)
{
	fmt_ll *entry = malloc(sizeof *entry);
	if (entry == NULL) {
		return STR_NO_MEM;
	}
	*entry = (fmt_ll){ .tname = tname, .print_fn = print_fn };

	if (fmt_hmap_insert(entry))
		return 0;
	return STR_ALREADY_EXISTS;
}

int fmt_deregister(str tname)
{
	fmt_ll *entry = fmt_hmap_lookup(tname);
	if (entry) {
		free(entry);
		return 0;
	}
	return STR_NOT_FOUND;
}

/* TODO */
