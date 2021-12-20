#ifndef INCLUDE_STR_FMT_H
#define INCLUDE_STR_FMT_H

#include <stdlib.h>
#include "str/str.h"

/* -- Config -- */
enum { FMT_HMAP_HASH_MULT = 31, FMT_HMAP_NBUCKETS = 257 };

enum FMT_ERRORS {FMT_NO_MEM = 200};

/* -- Data structures -- */

typedef struct fmt_ll {
	struct fmt_ll *next;
	str type_name;
	void (*print_fn)(void *);
} fmt_ll;

/* -- Functions -- */

fmt_ll* fmt_ll_lookup(str tn);

int fmt_init();
size_t fmt_hash(str s);
fmt_ll* fmt_hmap_lookup(str tn) {

/* -- Macros -- */

/* -- Constants -- */

extern fmt_hmap fmt_tnf_pair;

#endif
