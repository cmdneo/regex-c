#ifndef INCLUDE_STR_FMT_H
#define INCLUDE_STR_FMT_H

#include <stdlib.h>
#include "strlx/strlx.h"

/* -- Config -- */
enum {
	FMT_HMAP_HASH_MULT = 31,
	FMT_HMAP_NBUCKETS = 257,
};

enum FMT_ERRORS { FMT_NO_MEM = 200 };

/* -- Data structures -- */

typedef void (*printer)(void *);

typedef struct fmt_ll {
	struct fmt_ll *next;
	str tname;
	printer print_fn;
} fmt_ll;

/* -- Functions -- */

int fmt_init();

/* -- Macros -- */
#define print

/* -- Constants -- */

static fmt_ll *fmt_tnf[FMT_HMAP_NBUCKETS];

#endif
