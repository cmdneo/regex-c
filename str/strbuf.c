#include <stdio.h>
#include <stdlib.h>

#include "str/str.h"

strbuf strbuf_create(isize_t capacity)
{
	char *data = malloc(capacity);
	if (data == NULL)
		return (strbuf){ .error = STRLIB_NO_MEM };

	return (strbuf){ .data = data, .capacity = capacity, .size = 0, .error = 0 };
}

strbuf strbuf_create_from_str(str s)
{
	strbuf ret = strbuf_create(s.size);
	ret.size = s.size;
	if (ret.error)
		return ret;

	for (isize_t i = 0; i < s.size; i++)
		ret.data[i] = s.data[i];

	return ret;
}

strbuf strbuf_create_from_file(FILE *f, char end_marker)
{
	strbuf s = { 0 };
	int c;

	for (isize_t size = 1; (c = fgetc(f)) != EOF && c != end_marker; size++) {
		if (s.capacity >= size) {
			s.data[size - 1] = c;
			s.size = size;
			continue;
		}

		isize_t new_capacity = size * MEMORY_SCALE_FACTOR;
		strbuf_resize(&s, new_capacity);
		if (s.error)
			break;

		s.size = size;
		s.data[size - 1] = c;
	}

	return s;
}

strbuf strbuf_create_copy(strbuf const *s)
{
	strbuf ret = strbuf_create(s->capacity);
	ret.size = s->size;
	if (ret.error)
		return ret;

	for (isize_t i = 0; i < s->size; i++)
		ret.data[i] = s->data[i];

	return ret;
}

str strbuf_substr(strbuf const *s, isize_t start, isize_t end)
{
	if (!strlib_is_valid_range(s->size, start, end))
		return (str){ 0 };

	return (str){ .size = end - start, .data = s->data + start };
}

str strbuf_to_str(strbuf const *s)
{
	return (str){ .data = s->data, .size = s->size };
}

void strbuf_resize(strbuf *s, isize_t new_capacity)
{
	if (s->error)
		return;

	char *new_data = realloc(s->data, new_capacity);
	if (new_data == NULL) {
		s->error = STRLIB_NO_MEM;
		return;
	}

	s->capacity = new_capacity;
	s->data = new_data;
}

void strbuf_destroy(strbuf *s)
{
	free(s->data);
	*s = (strbuf){ 0 };
}

void strbuf_insert(strbuf *s, str t, isize_t pos)
{
	if (s->error)
		return;

	isize_t old_size = s->size;
	isize_t new_size = s->size + t.size;

	if (new_size > s->capacity) {
		/* allocate MEMORY_SCALE_FACTORx more memory than needed */
		strbuf_resize(s, new_size * MEMORY_SCALE_FACTOR);
		if (s->error)
			return;
	}

	/* Shift string from pos by t.size to make space for insertion */
	for (isize_t i = old_size - 1; i >= pos; i--)
		s->data[i + t.size] = s->data[i];
	/* Insert the substring at pos */
	for (isize_t i = 0; i < t.size; i++)
		s->data[i + pos] = t.data[i];

	s->size = new_size;
}

void strbuf_remove(strbuf *s, isize_t start, isize_t end)
{
	if (s->error)
		return;

	if (!strlib_is_valid_range(s->size, start, end)) {
		s->error = STRLIB_INVALID_INDEX;
		return;
	}

	isize_t substr_size = end - start;
	isize_t new_size = s->size - substr_size;

	for (isize_t i = start; i < new_size; i++)
		s->data[i] = s->data[i + substr_size];

	s->size = new_size;
}

void strbuf_replace(strbuf *s, str old, str new)
{
	if (s->error)
		return;

	isize_t substr_start = str_find_first(strbuf_to_str(s), old);
	while (substr_start >= 0) {
		strbuf_remove(s, substr_start, substr_start + old.size);
		strbuf_insert(s, new, substr_start);
		substr_start = str_find_first(strbuf_to_str(s), old);
	}
}

void strbuf_append(strbuf *s, str t)
{
	strbuf_insert(s, t, s->size);
}

void strbuf_prepend(strbuf *s, str t)
{
	strbuf_insert(s, t, 0);
}

void strbuf_reverse(strbuf *s)
{
	if (s->error)
		return;

	for (isize_t i = 0; i < s->size / 2; i++) {
		char tmp = s->data[i];
		s->data[i] = s->data[s->size - i - 1];
		s->data[s->size - i - 1] = tmp;
	}
}

void strbuf_to_upper(strbuf *s)
{
	if (s->error)
		return;

	for (isize_t i = 0; i < s->size; i++) {
		if ('a' <= s->data[i] && s->data[i] <= 'z')
			s->data[i] = s->data[i] + ('A' - 'a');
	}
}

void strbuf_to_lower(strbuf *s)
{
	if (s->error)
		return;

	for (isize_t i = 0; i < s->size; i++) {
		if ('A' <= s->data[i] && s->data[i] <= 'Z')
			s->data[i] = s->data[i] + ('a' - 'A');
	}
}

void strbuf_print(strbuf const *s)
{
	for (isize_t i = 0; i < s->size; i++)
		strlib_print_char(s->data[i]);
}
