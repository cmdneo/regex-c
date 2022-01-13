#ifndef INCLUDE_RE_RE_H
#define INCLUDE_RE_RE_H

#include "strlx/strlx.h"

/* -- Config -- */

enum re_metachars {
	RM_BKSLASH = '\\',
	RM_LBRACKET = '[',
	RM_RBRACKET = ']',
	RM_LBRACE = '{',
	RM_RBRACE = '}',
	RM_LPAREN = '(',
	RM_RPAREN = ')',
	RM_QMARK = '?',
	RM_CARET = '^',
	RM_DOLLAR = '$',
	RM_ASTERISK = '*',
	RM_PLUS = '+',
	RM_PERIOD = '.',
	RM_BAR = '|',
};

enum re_char_classes {
	RC_DIGIT = 'd',
	RC_NON_DIGIT = 'D',
	RC_WORD_CHAR = 'w',
	RC_NON_WORD_CHAR = 'W',
	RC_WHITESPACE = 's',
	RC_NON_WHITESPACE = 'S',

};

/* -- Data structures -- */

// typedef struct rrd {
// 	int nnodes;
// 	rrd *nodes;
// 	int ngroups;
// 	capgroup groups;
// } rrd;

// typedef struct capgroup {
// } capgroup;

// typedef struct re {
// 	str pattern;
// 	rrd cflow;
// } re;

// typedef struct matchg {
// 	isize start;
// 	isize end;
// 	str orig;
// 	str match;
// 	str gname;
// } matchg;

// /* -- Functions -- */
// re *re_parse(str pattern);
// matchg *re_starts_with(re *regex, str s);
// matchg *re_search(re *regex, str s)

#endif
