#ifndef INCLUDE_RE_RE_H
#define INCLUDE_RE_RE_H

#include "str/str.h"

/* -- Config -- */

enum RE_SPL_CHARS {
	RE_BKSLASH = '\\',
	RE_LBRACKET = '[',
	RE_RBRACKET = ']',
	RE_LBRACE = '{',
	RE_RBRACE = '}',
	RE_LPAREN = '(',
	RE_RPAREN = ')',
	RE_QMARK = '?',
	RE_CARET = '^',
	RE_DOLLAR = '$',
	RE_ASTERISK = '*',
	RE_PLUS = '+',
	RE_PERIOD = '.',
	RE_BAR = '|',
};

enum RE_SCC_CHARS {
	RE_DIGIT = 'd',
	RE_NON_DIGIT = 'D',
	RE_WORD_CHAR = 'w',
	RE_NON_WORD_CHAR = 'W',
	RE_WHITESPACE = 's',
	RE_NON_WHITESPACE = 'S',

};

/* -- Data structures -- */
/* TODO */

/* -- Functions -- */
/* TODO */
#endif