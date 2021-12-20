#include <stdlib.h>

#include "str/fmt.h"
#include "str/fmt.h"

static fmt_ll fmt_tnf_hmap[FMT_HMAP_NBUCKETS];

int fmt_init()
{
	return 0;
}

size_t fmt_hash(str s) {
	size_t h = 0;

	for (size_t i = 0; i < s.size; i++)
		h += s.data[i] * FMT_HMAP_HASH_MULT ;
	
	return h;
}


fmt_ll* fmt_hmap_lookup(str tn) {
	size_t h = fmt_hash(tn) % FMT_HMAP_NBUCKETS;
}