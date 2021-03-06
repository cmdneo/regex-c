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

#include <stdbool.h>
#include <stdio.h> /* FILE, EOF */

/* -- Enums -- */

enum strlx_error {
	STRLX_NO_ERR,
	STRLX_INVALID_INDEX,
	STRLX_NO_MEM,
	STRLX_NERRORS,
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
bool str_has_char(str s, char c);
bool str_has_char_case(str s, char c);

bool str_starts_with(str s, str t);
bool str_starts_with_case(str s, str t);

bool str_ends_with(str s, str t);
bool str_ends_with_case(str s, str t);

str str_remove_prefix(str s, str pref);
str str_remove_suffix(str s, str suff);
str str_lstrip(str s, str chars);
str str_rstrip(str s, str chars);
str str_strip(str s, str chars);

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
 * str can be prefixed with + or - sign.
 * If an illegal char is detected then parsing stops there.
 * NOTE: No prefixes allowed for numbers(like 0x for base 16).
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
bool strbuf_has_char(strbuf const *s, char c);
bool strbuf_starts_with(strbuf const *s, str t);
bool strbuf_ends_with(strbuf const *s, str t);

void strbuf_insert(strbuf *s, str t, isize pos);
void strbuf_remove(strbuf *s, isize start, isize end);

void strbuf_remove_prefix(strbuf *s, str pref);
void strbuf_remove_suffix(strbuf *s, str suff);
void strbuf_lstrip(strbuf *s, str chars);
void strbuf_rstrip(strbuf *s, str chars);
void strbuf_strip(strbuf *s, str chars);
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
 * @param s
 * @param base one of 2-36 (inclusive)
 * @param num Pointer to where the number will be stored
 * @return isize Index of the first illegal char. (s->size on success)
 */
isize strbuf_to_ll(strbuf const *s, int base, long long *num);
void strbuf_print(strbuf const *s);

/* Common Functions */

void strlx_adjust_range(isize size, isize *start, isize *end);

/* -- Macros -- */

/**
 * @brief Creates str from C-string literal, useful for consts
 *
 * Not a compound literal, cast to str if using for compound literal
 */
#define M_str(string_literal)                                                  \
	{                                                                      \
		.data = (string_literal), .size = (sizeof(string_literal) - 1) \
	}

/* One indirection needed for using the ## operator due to the way
   C preprocessor works, without it macro arguments are not expanded */
#define CONCAT_TOKENS_IMPL(a, b) a##b
#define CONCAT_TOKENS(a, b) CONCAT_TOKENS_IMPL(a, b)
#define MACRO_VAR(var) (CONCAT_TOKENS(var, __LINE__))

#define STR_FOREACH(s, cvar)                             \
	for (isize MACRO_VAR(i__) = 0;                   \
	     (MACRO_VAR(i__) < (s).size) &&              \
	     (((cvar) = (s).data[MACRO_VAR(i__)]) || 1); \
	     MACRO_VAR(i__)++)

#define strbuf_from(any) \
	_Generic((any),                    \
	int: strbuf_from_cap,              \
	isize: strbuf_from_cap,            \
	str: strbuf_from_str,              \
	strbuf *: strbuf_copy,             \
	char *: strbuf_from_cstr,          \
	char[sizeof(any)]: strbuf_from_cstr)(any)

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
static const str STR_PUNCT = M_str("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~");

/* -- Inline Charatcer functions -- */

static inline bool strlx_is_space(char c)
{
	for (isize i = 0; i < STR_WHITESPACES.size; i++)
		if (c == STR_WHITESPACES.data[i])
			return 1;

	return 0;
}

static inline bool strlx_is_upper(char c)
{
	return 'A' <= c && c <= 'Z';
}

static inline bool strlx_is_lower(char c)
{
	return 'a' <= c && c <= 'z';
}

static inline char strlx_to_lower(char c)
{
	if (strlx_is_upper(c))
		return c + ('a' - 'A');
	else
		return c;
}

static inline char strlx_to_upper(char c)
{
	if (strlx_is_lower(c))
		return c + ('A' - 'a');
	else
		return c;
}

static inline bool strlx_is_range_valid(isize size, isize start, isize end)
{
	return !(start > end || end > size || start < 0);
}

#endif /* END strlx/strlx.h */
