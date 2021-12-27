#ifndef INCLUDE_STR_STR_H
#define INCLUDE_STR_STR_H

/* For FILE type */
#include <stdio.h>

/* -- Config -- */

enum { MEMORY_SCALE_FACTOR = 2, STRBUF_INIT_CAP = 8 };

/* -- Errors -- */

enum strlib_errors {
	STRLIB_INVALID_INDEX = 100,
	STRLIB_NO_MEM,
	STRLIB_ALREADY_EXISTS,
	STRLIB_NOT_FOUND
};

/* -- Data structures -- */

typedef signed long long isize_t;
typedef void *(*allocator_fn)(void *ptr, isize_t size);

typedef struct str {
	isize_t size;
	char const *data;
} str;

typedef struct strbuf {
	char *data;
	isize_t capacity;
	isize_t size;
	int error;
} strbuf;

/* -- Functions -- */
/* Common Functions */

int strlib_is_valid_range(isize_t size, isize_t start, isize_t end);
void strlib_show_error_impl(int error);
void strlib_print_char(char c);

/* str functions */

str cstr(char const *s);
str str_substr(str s, isize_t start, isize_t end);
isize_t str_cmp(str s, str t);
isize_t str_find_first(str s, str substr);
isize_t str_find_last(str s, str substr);
int str_contains(str s, str substr);
str str_lstrip(str s, str substr);
str str_rstrip(str s, str substr);
str str_strip(str s, str substr);
str str_pop_first_split(str *s, str split_by);
void str_print(str s);

/* strbuf functions */

strbuf strbuf_create(isize_t capacity);
strbuf strbuf_create_from_str(str s);
strbuf strbuf_create_from_file(FILE *f, char end_marker);
strbuf strbuf_create_copy(strbuf const *s);
str strbuf_substr(strbuf const *s, isize_t start, isize_t end);
str strbuf_to_str(strbuf const *s);
isize_t strbuf_cmp(strbuf const *s, str t);
isize_t strbuf_find_first(strbuf const *s, str substr);
isize_t strbuf_find_last(strbuf const *s, str substr);
int strbuf_contains(strbuf const *s, str substr);
void strbuf_resize(strbuf *s, isize_t new_capacity);
void strbuf_destroy(strbuf *s);
void strbuf_insert(strbuf *s, str t, isize_t pos);
void strbuf_remove(strbuf *s, isize_t start, isize_t end);
void strbuf_lstrip(strbuf *s, str substr);
void strbuf_rstrip(strbuf *s, str substr);
void strbuf_strip(strbuf *s, str substr);
void strbuf_replace(strbuf *s, str old, str new);
void strbuf_append(strbuf *s, str t);
void strbuf_prepend(strbuf *s, str t);
void strbuf_reverse(strbuf *s);
void strbuf_to_upper(strbuf *s);
void strbuf_to_lower(strbuf *s);
void strbuf_print(strbuf const *s);

/* -- Macros -- */

#define PRI_isize_t "lli"
#define show_error(s)                                                                              \
	strlib_show_error_impl(_Generic((s), \
strbuf: (s).error,\
strbuf*: (*s).error,\
strbuf const*: (*s).error,\
strbuf const* const: (*s).error,\
))

/* -- Constants -- */

static const str STR_DIGITS = { 10, "0123456789" };
static const str STR_LOWERCASE = { 26, "abcdefghijklmnopqrstuvwxyz" };
static const str STR_UPPERCASE = { 26, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
static const str STR_LETTERS = { 52, "abcdefghijklmnopqrstuvwxyz"
				     "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
static str const STR_WHITESPACES = { 6, " \t\r\n\f\v" };
static const str STR_ALNUM = { 62, "0123456789"
				   "abcdefghijklmnopqrstuvwxyz"
				   "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };

#endif /* END str/str.h */
