#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "strlx/strlx.h"

enum strbuf_default {
	STRBUF_INIT_CAP = 8,
	MEMORY_SCALEX = 2,
};

static inline strbuf *strbuf_create_cap0()
{
	strbuf *ret = calloc(1, sizeof *ret);
	if (ret == NULL)
		return NULL;
	*ret = (strbuf){ 0 };

	return ret;
}

static void strbuf_shift(strbuf *s, isize offset, isize shift_by)
{
	assert(s);
	assert(offset <= s->size);

	if (shift_by == 0)
		return;
	if (shift_by > 0 && (s->cap < s->size + shift_by)) {
		strbuf_resize(s, s->cap * MEMORY_SCALEX);
		if (s->error)
			return;
	}

	if (shift_by > 0) {
		for (isize i = s->size - 1; i >= offset; i--)
			s->data[i + shift_by] = s->data[i];
	} else {
		assert(llabs(shift_by) <= offset);
		for (isize i = offset; i < s->size; i++)
			s->data[i + shift_by] = s->data[i];
	}

	s->size = s->size + shift_by;
}

strbuf *strbuf_from_cap(isize cap)
{
	assert(cap > 0);

	strbuf *ret = strbuf_create_cap0();
	if (ret == NULL)
		return NULL;
	char *data = malloc(cap);
	if (data == NULL) {
		free(ret);
		return NULL;
	}

	*ret = (strbuf){
		.data = data,
		.cap = cap,
		.size = 0,
		.error = 0,
	};

	return ret;
}

strbuf *strbuf_from_str(str s)
{
	isize cap = s.size > 0 ? s.size : STRBUF_INIT_CAP;
	strbuf *ret = strbuf_from_cap(cap);
	if (ret == NULL)
		return NULL;

	ret->size = s.size;
	for (isize i = 0; i < s.size; i++)
		ret->data[i] = s.data[i];

	return ret;
}

strbuf *strbuf_from_cstr(char const *s)
{
	return strbuf_from_str(cstr(s));
}

strbuf *strbuf_from_file(FILE *f)
{
	assert(f);

	strbuf *ret = strbuf_from_cap(STRBUF_INIT_CAP);
	if (ret == NULL)
		return NULL;

	int c;
	isize size;
	for (size = 0; (c = fgetc(f)) != EOF; size++) {
		if (ret->cap >= size) {
			ret->data[size - 1] = c;
			ret->size = size;
			continue;
		}

		isize new_capacity = size * MEMORY_SCALEX;
		strbuf_resize(ret, new_capacity);
		if (ret->error)
			break;

		ret->data[size] = c;
	}
	ret->size = size;

	return ret;
}

strbuf *strbuf_copy(strbuf const *s)
{
	assert(s);
	if (s->error)
		return NULL;
	strbuf *ret = strbuf_from_cap(s->size);
	if (ret == NULL)
		return NULL;

	ret->size = s->size;
	for (isize i = 0; i < s->size; i++)
		ret->data[i] = s->data[i];

	return ret;
}

str strbuf_substr(strbuf const *s, isize start, isize end)
{
	assert(s);
	if (s->error || !strlx_is_valid_range(s->size, start, end))
		return (str){ .size = 0 };

	return (str){ .size = end - start, .data = s->data + start };
}

str strbuf_to_str(strbuf const *s)
{
	assert(s);
	if (s->error)
		return (str){ 0 };

	return (str){ .data = s->data, .size = s->size };
}

isize strbuf_cmp(strbuf const *s, str t)
{
	assert(s);
	return str_cmp(strbuf_to_str(s), t);
}

isize strbuf_cmp2(strbuf const *s, strbuf const *t)
{
	assert(s);
	return str_cmp(strbuf_to_str(s), strbuf_to_str(t));
}

isize strbuf_find_first(strbuf const *s, str substr)
{
	assert(s);
	return str_find_first(strbuf_to_str(s), substr);
}

isize strbuf_find_last(strbuf const *s, str substr)
{
	assert(s);
	return str_find_last(strbuf_to_str(s), substr);
}

isize strbuf_count(strbuf const *s, str t)
{
	assert(s);
	return str_count(strbuf_to_str(s), t);
}

int strbuf_has_char(strbuf const *s, char c)
{
	assert(s);
	return str_has_char(strbuf_to_str(s), c);
}

int strbuf_starts_with(strbuf const *s, str t)
{
	assert(s);
	return str_starts_with(strbuf_to_str(s), t);
}
int strbuf_ends_with(strbuf const *s, str t)
{
	assert(s);
	return str_ends_with(strbuf_to_str(s), t);
}

void strbuf_resize(strbuf *s, isize new_capacity)
{
	assert(s);
	assert(new_capacity > 0);
	if (s->error)
		return;

	char *new_data = realloc(s->data, new_capacity);
	if (new_data == NULL) {
		s->error = STRLX_NO_MEM;
		return;
	}

	s->cap = new_capacity;
	s->data = new_data;
}

void strbuf_destroy(strbuf **s)
{
	strbuf *tmp = *s;
	assert(s);
	assert(tmp);
	assert(tmp->data);

	free(tmp->data);
	free(tmp);
	*s = NULL;
}

void strbuf_insert(strbuf *s, str t, isize pos)
{
	assert(s);
	if (s->error)
		return;

	strbuf_shift(s, pos, t.size);
	if (s->error)
		return;
	/* Insert the substring at pos */
	for (isize i = 0; i < t.size; i++)
		s->data[i + pos] = t.data[i];
}
void strbuf_remove(strbuf *s, isize start, isize end)
{
	assert(s);
	if (s->error)
		return;
	if (!strlx_is_valid_range(s->size, start, end)) {
		s->error = STRLX_INVALID_INDEX;
		return;
	}

	isize sub_size = end - start;
	strbuf_shift(s, end, -sub_size);
}

void strbuf_lstrip(strbuf *s, str substr)
{
	assert(s);
	if (s->error)
		return;

	str tmp = str_lstrip(strbuf_to_str(s), substr);
	strbuf_remove(s, 0, (isize)(tmp.data - s->data));
}

void strbuf_rstrip(strbuf *s, str substr)
{
	assert(s);
	if (s->error)
		return;

	str tmp = str_rstrip(strbuf_to_str(s), substr);
	s->size = tmp.size;
}

void strbuf_strip(strbuf *s, str substr)
{
	assert(s);
	strbuf_lstrip(s, substr);
	strbuf_rstrip(s, substr);
}

isize strbuf_replace(strbuf *s, str old, str new)
{
	assert(s);
	if (s->error)
		return -1;

	isize count = 0;
	isize substr_start = strbuf_find_first(s, old);
	while (substr_start >= 0) {
		strbuf_remove(s, substr_start, substr_start + old.size);
		strbuf_insert(s, new, substr_start);
		substr_start = strbuf_find_first(s, old);
		count++;
	}

	return count;
}

void strbuf_append(strbuf *s, str t)
{
	assert(s);
	strbuf_insert(s, t, s->size);
}

void strbuf_prepend(strbuf *s, str t)
{
	assert(s);
	strbuf_insert(s, t, 0);
}

void strbuf_ljust(strbuf *s, char fill, isize width)
{
	assert(s);
	if (s->error)
		return;

	isize sft = width - s->size;
	if (sft <= 0)
		return;

	strbuf_shift(s, 0, sft);
	if (s->error)
		return;
	/* Fill characters */
	for (isize i = 0; i < sft; i++)
		s->data[i] = fill;
}

void strbuf_rjust(strbuf *s, char fill, isize width)
{
	assert(s);
	if (s->error)
		return;

	isize sft = width - s->size;
	if (sft <= 0)
		return;

	strbuf_shift(s, s->size, sft);
	if (s->error)
		return;
	/* Fill characters */
	for (isize i = s->size - sft; i < s->size; i++)
		s->data[i] = fill;
}

void strbuf_center(strbuf *s, char fill, isize width)
{
	assert(s);
	if (s->error)
		return;

	isize sft = width - s->size;
	isize sft_l = sft / 2;
	isize sft_r = sft - sft_l;

	strbuf_ljust(s, fill, sft_l + s->size);
	strbuf_rjust(s, fill, sft_r + s->size);
}

void strbuf_reverse(strbuf *s)
{
	assert(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size / 2; i++) {
		char tmp = s->data[i];
		s->data[i] = s->data[s->size - i - 1];
		s->data[s->size - i - 1] = tmp;
	}
}

void strbuf_to_upper(strbuf *s)
{
	assert(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size; i++) {
		if ('a' <= s->data[i] && s->data[i] <= 'z')
			s->data[i] = s->data[i] + ('A' - 'a');
	}
}

void strbuf_to_lower(strbuf *s)
{
	assert(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size; i++) {
		if ('A' <= s->data[i] && s->data[i] <= 'Z')
			s->data[i] = s->data[i] + ('a' - 'A');
	}
}

void strbuf_print(strbuf const *s)
{
	assert(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size; i++)
		putchar(s->data[i]);
}
