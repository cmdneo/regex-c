/**
 * @file strlx.h
 * @brief String library 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INCLUDE_STRLX_STRLX_H_
#define INCLUDE_STRLX_STRLX_H_

#include <stdio.h> /* FILE, EOF */

/* -- Config -- */

enum strlx_error {
	STRLX_NO_ERR = 0,
	STRLX_INVALID_INDEX = 100,
	STRLX_NO_MEM,
	STRLX_ALREADY_EXISTS,
	STRLX_NOT_FOUND,
};

/* -- Data structures and functions -- */

typedef long isize;

typedef struct str {
	isize size;
	char const *data;
} str;

str cstr(char const *s);
str str_substr(str s, isize start, isize end);

isize str_cmp(str s, str t);
isize str_cmp_case(str s, str t);
/**
 * @brief Reverses s then compares s and t
 * 
 * @param s
 * @param t
 * @return int Number of characters matched
 */
isize str_cmp_rev(str s, str t);

isize str_find_first(str s, str t);
isize str_find_last(str s, str t);

isize str_count(str s, str t);
int str_has_char(str s, char c);
int str_has_char_case(str s, char c);

int str_starts_with(str s, str t);
int str_starts_with_case(str s, str t);

int str_ends_with(str s, str t);
int str_ends_with_case(str s, str t);

str str_lstrip(str s, str t);
str str_rstrip(str s, str t);
str str_strip(str s, str t);

/**
 * @brief Splits and pops string one by one.
 *
 * If delimeter is absent in the string then the full string is
 * returned and the original string becomes empty
 *
 * @param s the first delimiter and the chars before it are removed
 * @param split_by the delimiter
 * @return str the popped substring
 */
str str_pop_first_split(str *s, str split_by);

/**
 * @brief Converts str to long long int.
 * str can be have any number of leading whitespaces, can be prefixed with
 * + or - sign and can have prefix hexadecimal(0x and 0X) integers
 * just after the sign.
 * If an illegal char is detected then parsing stops there
 * 
 * @param s 
 * @param base one of 2-36 (inclusive)
 * @param num Pointer to where the number will be stored
 * @return isize Index of first illegal char. (s.size on success)
 */
isize str_to_ll(str s, int base, long long *num);
void str_print(str s);

typedef struct strbuf {
	char *data;
	isize cap;
	isize size;
	int error;
} strbuf;

strbuf *strbuf_from_cap(isize cap);
strbuf *strbuf_from_str(str s);
strbuf *strbuf_from_cstr(char const *s);
strbuf *strbuf_from_file(FILE *f, char end);
strbuf *strbuf_copy(strbuf const *s);
void strbuf_resize(strbuf *s, isize new_capacity);
void strbuf_destroy(strbuf **s);

str strbuf_substr(strbuf const *s, isize start, isize end);
str strbuf_to_str(strbuf const *s);

isize strbuf_cmp(strbuf const *s, str t);
isize strbuf_cmp2(strbuf const *s, strbuf const *t);
isize strbuf_find_first(strbuf const *s, str t);
isize strbuf_find_last(strbuf const *s, str t);
isize strbuf_count(strbuf const *s, str t);
int strbuf_has_char(strbuf const *s, char c);
int strbuf_starts_with(strbuf const *s, str t);
int strbuf_ends_with(strbuf const *s, str t);

void strbuf_insert(strbuf *s, str t, isize pos);
void strbuf_remove(strbuf *s, isize start, isize end);
void strbuf_lstrip(strbuf *s, str t);
void strbuf_rstrip(strbuf *s, str t);
void strbuf_strip(strbuf *s, str t);
isize strbuf_replace(strbuf *s, str old, str new);
void strbuf_append(strbuf *s, str t);
void strbuf_prepend(strbuf *s, str t);

void strbuf_ljust(strbuf *s, char fill, isize width);
void strbuf_rjust(strbuf *s, char fill, isize width);
void strbuf_center(strbuf *s, char fill, isize width);

void strbuf_reverse(strbuf *s);
void strbuf_to_upper(strbuf *s);
void strbuf_to_lower(strbuf *s);

/**
 * @brief Converts str to long long, strbuf wrapper for str_to_ll.
 * 
 * * @param s 
 * @param base one of 2-36 (inclusive)
 * @param num Pointer to where the number will be stored
 * @return isize Index of the first illegal char. (s->size on success)
 */
isize strbuf_to_ll(strbuf const *s, int base, long long *num);
void strbuf_print(strbuf const *s);

/* Common Functions */

void strlx_adjust_range(isize size, isize *start, isize *end);
int strlx_is_range_valid(isize size, isize start, isize end);
void strlx_show_error(enum strlx_error error);

/* -- Macros -- */

/** Creates str from C-string literal, useful for consts */
#define M_str(string_literal)                         \
	((str){                                       \
		.data = (string_literal),             \
		.size = (sizeof(string_literal) - 1), \
	})

/* One indirection needed for using the ## operator due to the way
   C preprocessor works, without it macro arguments are not expanded */
#define CONCAT_TOKENS_IMPL(a, b) a##b
#define CONCAT_TOKENS(a, b) CONCAT_TOKENS_IMPL(a, b)
#define MACRO_VAR(var) (CONCAT_TOKENS(var, __LINE__))

#define STR_FOREACH(s, cvar)                               \
	for (isize MACRO_VAR(i__) = 0;                     \
	     MACRO_VAR(i__) < (s).size &&                  \
	     (((cvar) = (s).data[MACRO_VAR(i__)]) != EOF); \
	     MACRO_VAR(i__)++)

#define strbuf_from(any) \
	_Generic((any),                    \
	int: strbuf_from_cap,              \
	/* Here isize is long int */       \
	isize: strbuf_from_cap,            \
	str: strbuf_from_str,              \
	strbuf*: strbuf_copy,              \
	char*: strbuf_from_cstr,           \
	/* For string literals: char[N] */ \
	char[sizeof any]: strbuf_from_cstr \
)(any)

#define PRI_isize "ld"

/* -- Constants -- */

static const str STR_NUL = M_str("\0");
static const str STR_LOWERCASE = M_str("abcdefghijklmnopqrstuvwxyz");
static const str STR_UPPERCASE = M_str("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const str STR_LETTERS = M_str("abcdefghijklmnopqrstuvwxyz"
				     "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const str STR_WHITESPACES = M_str(" \t\r\n\f\v");
static const str STR_ALNUM = M_str("0123456789"
				   "abcdefghijklmnopqrstuvwxyz"
				   "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const str STR_BIN_DIGITS = M_str("01");
static const str STR_OCT_DIGITS = M_str("01234567");
static const str STR_DIGITS = M_str("0123456789");
static const str STR_HEX_DIGITS = M_str("0123456789abcdefABCDEF");
static const str STR_PUNCT = M_str("!\"#$%%&\'()*+,-./:;<=>?@[\\]^_`{|}~");

/* -- Inline Charatcer functions -- */

static inline int strlx_is_space(char c)
{
	for (isize i = 0; i < STR_WHITESPACES.size; i++)
		if (c == STR_WHITESPACES.data[i])
			return 1;

	return 0;
}

static inline int strlx_is_upper(char c)
{
	return 'A' <= c && c <= 'Z';
}

static inline int strlx_is_lower(char c)
{
	return 'a' <= c && c <= 'z';
}

static inline char strlx_to_lower(char c)
{
	return c + ('a' - 'A') * strlx_is_upper(c);
}

static inline char strlx_to_upper(char c)
{
	return c + ('A' - 'a') * strlx_is_lower(c);
}

#endif /* END strlx/strlx.h */
