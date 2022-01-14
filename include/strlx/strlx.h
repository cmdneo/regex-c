/**
 * @file strlx.h
 * @brief String library 
 * @version 0.1
 * @date 2022-01-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INCLUDE_STRLX_STR_H_
#define INCLUDE_STRLX_STR_H_

/* For FILE type */
#include <stdio.h>

/* -- Config -- */

enum str_default {
	MEMORY_SCALE_FACTOR = 2,
	STRBUF_INIT_CAP = 8,
};

enum str_error {
	STR_OK = 0,
	STR_INVALID_INDEX = 100,
	STR_NO_MEM,
	STR_MEM_FREED,
	STR_ALREADY_EXISTS,
	STR_NOT_FOUND,
};

/* -- Data structures and functions -- */

typedef long isize;

typedef struct str {
	isize size;
	char const *data;
} str;

/** Creates str from C-string literal, useful for consts.*/
#define M_str(string_literal)                                                  \
	(str)                                                                  \
	{                                                                      \
		.data = (string_literal), .size = (sizeof(string_literal) - 1) \
	}
str cstr(char const *s);
str str_substr(str s, isize start, isize end);
isize str_cmp(str s, str t);
isize str_find_first(str s, str t);
isize str_find_last(str s, str t);
isize str_count(str s, str t);
int str_starts_with(str s, str t);
int str_ends_with(str s, str t);
str str_lstrip(str s, str t);
str str_rstrip(str s, str t);
str str_strip(str s, str t);

/**
 * @brief Splits and pops string one by one.
 *        If delimeter is absent in the string then the full string is
 *        returned and the original string becomes empty
 *
 * @param s the first delimiter and the chars before it are removed
 * @param split_by the delimiter
 * @return str the popped substring
 */
str str_pop_first_split(str *s, str split_by);
void str_print(str s);

typedef struct strbuf {
	char *data;
	isize capacity;
	isize size;
	int error;
} strbuf;

strbuf *strbuf_create_empty();
strbuf *strbuf_from_cap(isize capacity);
strbuf *strbuf_from_str(str s);
strbuf *strbuf_from_cstr(char const *s);
strbuf *strbuf_from_file(FILE *f);
strbuf *strbuf_copy(strbuf const *s);
str strbuf_substr(strbuf const *s, isize start, isize end);
str strbuf_to_str(strbuf const *s);
isize strbuf_cmp(strbuf const *s, str t);
isize strbuf_cmp2(strbuf const *s, strbuf const *t);
isize strbuf_find_first(strbuf const *s, str t);
isize strbuf_find_last(strbuf const *s, str t);
isize strbuf_count(strbuf const *s, str t);
int strbuf_starts_with(strbuf *s, str t);
int strbuf_ends_with(strbuf *s, str t);
void strbuf_resize(strbuf *s, isize new_capacity);
void strbuf_destroy(strbuf **s);
void strbuf_insert(strbuf *s, str t, isize pos);
void strbuf_remove(strbuf *s, isize start, isize end);
void strbuf_lstrip(strbuf *s, str t);
void strbuf_rstrip(strbuf *s, str t);
void strbuf_strip(strbuf *s, str t);
void strbuf_replace(strbuf *s, str old, str new);
void strbuf_append(strbuf *s, str t);
void strbuf_prepend(strbuf *s, str t);
void strbuf_reverse(strbuf *s);
void strbuf_to_upper(strbuf *s);
void strbuf_to_lower(strbuf *s);
void strbuf_print(strbuf const *s);

/* Common Functions */

int str_is_valid_range(isize size, isize start, isize end);
void str_show_error(enum str_error error);

/* -- Macros -- */

#define strbuf_from(in_any)                                                    \
	_Generic((in_any),                    \
	int: strbuf_from_cap,                 \
	unsigned int: strbuf_from_cap,        \
	/* Here isize is long int */        \
	isize: strbuf_from_cap,             \
	str: strbuf_from_str,                 \
	strbuf*: strbuf_copy,                 \
	FILE*: strbuf_from_file,         \
	char*: strbuf_from_cstr,              \
	/* For string literals: char[N] */    \
	char[sizeof in_any]: strbuf_from_cstr \
)(in_any)

#define PRI_isize_t "ld"

/* -- Constants -- */

static const str STR_DIGITS = M_str("0123456789");
static const str STR_LOWERCASE = M_str("abcdefghijklmnopqrstuvwxyz");
static const str STR_UPPERCASE = M_str("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const str STR_LETTERS = M_str("abcdefghijklmnopqrstuvwxyz"
				     "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const str STR_WHITESPACES = M_str(" \t\r\n\f\v");
static const str STR_ALNUM = M_str("0123456789"
				   "abcdefghijklmnopqrstuvwxyz"
				   "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

#endif /* END str/str.h */
