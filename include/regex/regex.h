#ifndef INCLUDE_REGEX_REGEX_H
#define INCLUDE_REGEX_REGEX_H

#include "strlx/strlx.h"

#include "regex/errors.h" /* Export */

/* -- Config -- */

/* -- Data structures -- */
typedef struct egraph *egraph;
typedef struct regex *regex;

typedef struct regex_match {
	str gname;
	struct {
		isize start;
		isize end;
	} span;
	str match;
	regex pattern;
} regex_match;

/* -- Functions -- */
egraph re_parse(strbuf const *pattern);


/* -- Macros -- */

#endif
