#include <stdio.h>
#include <stdlib.h>

#include "str/str.h"

strbuf strbuf_create(isize_t capacity)
{
	char *data = malloc(capacity);
	if (data == NULL)
		return (strbuf){ .error = STR_NO_MEM };
	return (strbuf){
		.data = data, .capacity = capacity, .size = 0, .error = 0
	};
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
	strbuf s = strbuf_create(STRBUF_INIT_CAP);
	if (s.error)
		return s;

	int c;
	for (isize_t size = 1; (c = fgetc(f)) != EOF && c != end_marker;
	     size++) {
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
	if (s->error)
		return (strbuf){ .error = s->error };
	strbuf ret = strbuf_create(s->capacity);
	if (ret.error)
		return ret;

	ret.size = s->size;
	for (isize_t i = 0; i < s->size; i++)
		ret.data[i] = s->data[i];

	return ret;
}

str strbuf_substr(strbuf const *s, isize_t start, isize_t end)
{
	if (!str_is_valid_range(s->size, start, end))
		return (str){ 0 };

	return (str){ .size = end - start, .data = s->data + start };
}

str strbuf_to_str(strbuf const *s)
{
	if (s->error)
		return (str){ 0 };
	return (str){ .data = s->data, .size = s->size };
}

isize_t strbuf_find_first(strbuf const *s, str substr)
{
	return str_find_first(strbuf_to_str(s), substr);
}

isize_t strbuf_find_last(strbuf const *s, str substr)
{
	return str_find_last(strbuf_to_str(s), substr);
}

isize_t strbuf_cmp(strbuf const *s, str t)
{
	return str_cmp(strbuf_to_str(s), t);
}

isize_t strbuf_cmp2(strbuf const *s, strbuf const *t)
{
	return str_cmp(strbuf_to_str(s), strbuf_to_str(t));
}

isize_t strbuf_count(strbuf const *s, str t)
{
	return str_count(strbuf_to_str(s), t);
}

void strbuf_resize(strbuf *s, isize_t new_capacity)
{
	if (s->error)
		return;

	char *new_data = realloc(s->data, new_capacity);
	if (new_data == NULL) {
		s->error = STR_NO_MEM;
		return;
	}

	s->capacity = new_capacity;
	s->data = new_data;
}

void strbuf_destroy(strbuf *s)
{
	free(s->data);
	*s = (strbuf){ .error = STR_MEM_FREED };
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
	if (!str_is_valid_range(s->size, start, end)) {
		s->error = STR_INVALID_INDEX;
		return;
	}

	isize_t substr_size = end - start;
	isize_t new_size = s->size - substr_size;
	/* Shift string segment to the left by size of the removes substring */
	for (isize_t i = start; i < new_size; i++)
		s->data[i] = s->data[i + substr_size];

	s->size = new_size;
}

void strbuf_lstrip(strbuf *s, str substr)
{
	if (s->error)
		return;

	str tmp = str_lstrip(strbuf_to_str(s), substr);
	strbuf_remove(s, 0, (isize_t)(tmp.data - s->data));
}

void strbuf_rstrip(strbuf *s, str substr)
{
	if (s->error)
		return;

	str tmp = str_rstrip(strbuf_to_str(s), substr);
	s->size = tmp.size;
}

void strbuf_strip(strbuf *s, str substr)
{
	strbuf_lstrip(s, substr);
	strbuf_rstrip(s, substr);
}

void strbuf_replace(strbuf *s, str old, str new)
{
	if (s->error)
		return;

	isize_t substr_start = strbuf_find_first(s, old);
	while (substr_start >= 0) {
		strbuf_remove(s, substr_start, substr_start + old.size);
		strbuf_insert(s, new, substr_start);
		substr_start = strbuf_find_first(s, old);
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
	if (s->error)
		return;

	for (isize_t i = 0; i < s->size; i++)
		putchar(s->data[i]);
}
