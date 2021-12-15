#ifndef INCLUDE_STR_STR_H
#define INCLUDE_STR_STR_H

/* For FILE type */
#include <stdio.h>

/* -- Data and Config -- */

enum { MEMORY_SCALE_FACTOR = 2 };

/* -- Errors -- */

enum strlib_errors { STRLIB_INVALID_INDEX = 100, STRLIB_NO_MEM = 200 };

/* -- Data structures -- */

typedef signed long long isize_t;
typedef void *(*allocator_fn)(void *ptr, isize_t size);

typedef struct str {
	isize_t size;
	char const *data;
} str;

typedef struct strbuf {
	isize_t capacity;
	isize_t size;
	char *data;
	int error;
} strbuf;

/* -- Functions -- */
/* Common Functions */

void strlib_show_error_impl(int error);
void strlib_print_char(char c);

/* str functions */

str cstr(char const *s);
str str_substr(str s, isize_t start, isize_t end);
isize_t str_cmp(str s, str t);
isize_t str_find_first(str s, str substr);
isize_t str_find_last(str s, str substr);
str str_pop_first_split(str *s, str split_by);
void str_print(str s);

/* strbuf functions */

strbuf strbuf_create(isize_t capacity);
strbuf strbuf_create_from_str(str s);
strbuf strbuf_create_from_file(FILE *f, char end_marker);
strbuf strbuf_create_copy(strbuf const *s);
str strbuf_to_str(strbuf const *s);
void strbuf_resize(strbuf *s, isize_t new_capacity);
void strbuf_destroy(strbuf *s);
void strbuf_insert(strbuf *s, str t, isize_t pos);
void strbuf_remove(strbuf *s, isize_t start, isize_t end);
void strbuf_replace(strbuf *s, str old, str new);
void strbuf_append(strbuf *s, str t);
void strbuf_prepend(strbuf *s, str t);
void strbuf_reverse(strbuf *s);
void strbuf_to_upper(strbuf *s);
void strbuf_to_lower(strbuf *s);
void strbuf_print(strbuf const *s);

/* -- Macros -- */

#define show_error(s) strlib_show_error_impl(s.error)

#endif /* END str/str.h */
