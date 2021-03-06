#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "strlx/strlx.h"

#define assert_strbuf(s) \
	(assert((s)), assert((s)->data), assert((s)->size <= (s)->cap))

enum strbuf_default {
	STRBUF_INIT_CAP = 8,
};

static inline isize next_cap(isize cap)
{
	if (cap < 2)
		cap = 2;
	return cap + cap / 2;
}

static inline strbuf *strbuf_create_cap0()
{
	strbuf *ret = calloc(1, sizeof *ret);
	if (ret == NULL)
		return NULL;
	*ret = (strbuf){ 0 };

	assert(ret);
	return ret;
}

static void strbuf_auto_shrink(strbuf *s)
{
	assert_strbuf(s);

	if (s->cap > s->size * 3) {
		strbuf_resize(s, next_cap(s->size));
		// Error(if any) is set by strbuf_resize
	}

	assert_strbuf(s);
}

static void strbuf_shift(strbuf *s, isize offset, isize shift_by)
{
	assert_strbuf(s);
	assert(offset <= s->size);

	if (shift_by == 0)
		return;

	isize new_size = s->size + shift_by;
	if (s->cap < new_size) {
		strbuf_resize(s, next_cap(new_size));
		if (s->error)
			return;
	}

	if (shift_by > 0) {
		for (isize i = s->size - 1; i >= offset; i--)
			s->data[i + shift_by] = s->data[i];
	} else {
		assert(labs(shift_by) <= offset);
		for (isize i = offset; i < s->size; i++)
			s->data[i + shift_by] = s->data[i];
	}

	s->size = new_size;
	assert_strbuf(s);
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

	assert_strbuf(ret);
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

	assert_strbuf(ret);
	return ret;
}

strbuf *strbuf_from_cstr(char const *s)
{
	return strbuf_from_str(cstr(s));
}

strbuf *strbuf_from_file(FILE *f, char end)
{
	assert(f);

	strbuf *ret = strbuf_from_cap(STRBUF_INIT_CAP);
	if (ret == NULL)
		return NULL;

	int c;
	for (isize i = 0; (c = fgetc(f)) != EOF && c != end; i++) {
		if (i < ret->cap) {
			ret->data[i] = c;
			ret->size++;
			continue;
		}

		strbuf_resize(ret, next_cap(ret->cap));
		if (ret->error) {
			strbuf_destroy(&ret);
			return NULL;
		}

		ret->data[i] = c;
		ret->size++;
	}

	assert_strbuf(ret);
	return ret;
}

strbuf *strbuf_copy(strbuf const *s)
{
	assert_strbuf(s);
	if (s->error)
		return NULL;
	strbuf *ret = strbuf_from_cap(s->size);
	if (ret == NULL)
		return NULL;

	ret->size = s->size;
	for (isize i = 0; i < s->size; i++)
		ret->data[i] = s->data[i];

	assert_strbuf(ret);
	return ret;
}

void strbuf_resize(strbuf *s, isize new_capacity)
{
	assert_strbuf(s);
	assert(new_capacity > 0);
	if (s->error)
		return;

	char *new_data = realloc(s->data, sizeof(char[new_capacity]));
	if (new_data == NULL) {
		s->error = STRLX_NO_MEM;
		return;
	}

	s->cap = new_capacity;
	s->data = new_data;

	assert_strbuf(s);
}

void strbuf_destroy(strbuf **sp)
{
	assert(sp);
	strbuf *tmp = *sp;
	assert(tmp);
	assert(tmp->data);

	free(tmp->data);
	free(tmp);
	*sp = NULL;
}

str strbuf_substr(strbuf const *s, isize start, isize end)
{
	assert_strbuf(s);
	if (s->error)
		return (str){ .size = 0 };

	strlx_adjust_range(s->size, &start, &end);

	return (str){ .size = end - start, .data = s->data + start };
}

str strbuf_to_str(strbuf const *s)
{
	assert_strbuf(s);
	if (s->error)
		return (str){ 0 };

	return (str){ .data = s->data, .size = s->size };
}

isize strbuf_cmp(strbuf const *s, str t)
{
	assert_strbuf(s);
	return str_cmp(strbuf_to_str(s), t);
}

isize strbuf_cmp2(strbuf const *s, strbuf const *t)
{
	assert_strbuf(s);
	return str_cmp(strbuf_to_str(s), strbuf_to_str(t));
}

isize strbuf_find_first(strbuf const *s, str substr)
{
	assert_strbuf(s);
	return str_find_first(strbuf_to_str(s), substr);
}

isize strbuf_find_last(strbuf const *s, str substr)
{
	assert_strbuf(s);
	return str_find_last(strbuf_to_str(s), substr);
}

isize strbuf_count(strbuf const *s, str t)
{
	assert_strbuf(s);
	return str_count(strbuf_to_str(s), t);
}

bool strbuf_has_char(strbuf const *s, char c)
{
	assert_strbuf(s);
	return str_has_char(strbuf_to_str(s), c);
}

bool strbuf_starts_with(strbuf const *s, str t)
{
	assert_strbuf(s);
	return str_starts_with(strbuf_to_str(s), t);
}

bool strbuf_ends_with(strbuf const *s, str t)
{
	assert_strbuf(s);
	return str_ends_with(strbuf_to_str(s), t);
}

void strbuf_insert(strbuf *s, str t, isize pos)
{
	assert_strbuf(s);
	if (s->error)
		return;

	strbuf_shift(s, pos, t.size);
	if (s->error)
		return;
	/* Insert the substring at pos */
	for (isize i = 0; i < t.size; i++)
		s->data[i + pos] = t.data[i];

	assert_strbuf(s);
}
void strbuf_remove(strbuf *s, isize start, isize end)
{
	assert_strbuf(s);
	if (s->error)
		return;
	if (!strlx_is_range_valid(s->size, start, end)) {
		s->error = STRLX_INVALID_INDEX;
		return;
	}

	isize sub_size = end - start;
	strbuf_shift(s, end, -sub_size);
	strbuf_auto_shrink(s);

	assert_strbuf(s);
}

void strbuf_remove_prefix(strbuf *s, str pref)
{
	assert_strbuf(s);
	if (s->error)
		return;

	str tmp = str_remove_prefix(strbuf_to_str(s), pref);
	strbuf_remove(s, 0, (isize)(s->size - tmp.size));

	assert_strbuf(s);
}

void strbuf_remove_suffix(strbuf *s, str suff)
{
	assert_strbuf(s);
	if (s->error)
		return;

	str tmp = str_remove_suffix(strbuf_to_str(s), suff);
	strbuf_remove(s, tmp.size, s->size);

	assert_strbuf(s);
}

void strbuf_lstrip(strbuf *s, str chars)
{
	assert_strbuf(s);
	if (s->error)
		return;

	str tmp = str_lstrip(strbuf_to_str(s), chars);
	strbuf_remove(s, 0, (isize)(s->size - tmp.size));

	assert_strbuf(s);
}

void strbuf_rstrip(strbuf *s, str chars)
{
	assert_strbuf(s);
	if (s->error)
		return;

	str tmp = str_rstrip(strbuf_to_str(s), chars);
	strbuf_remove(s, tmp.size, s->size);

	assert_strbuf(s);
}

void strbuf_strip(strbuf *s, str chars)
{
	assert_strbuf(s);
	if (s->error)
		return;

	strbuf_rstrip(s, chars);
	strbuf_lstrip(s, chars);

	assert_strbuf(s);
}

isize strbuf_replace(strbuf *s, str old, str new)
{
	assert_strbuf(s);
	if (s->error)
		return -1;

	isize count = 0;
	str cp = strbuf_substr(s, 0, s->size);
	isize old_at = str_find_last(cp, old);

	while (old_at >= 0 && cp.size >= 0) {
		/* Make a space of size new.size at old_at to insert new */
		strbuf_shift(s, old_at + old.size, new.size - old.size);
		if (s->error)
			return -1;

		for (isize i = 0; i < new.size; i++)
			s->data[old_at + i] = new.data[i];

		/* Keep cutting off the string from where old was found(old_at)
		 * last time and if old.size==0 (empty str) then old_at will be
		 * cp.size, so subtract 1 from it(otherwise will cause an inf loop).
		 */
		cp.size = old_at - 1 * (old.size == 0);
		old_at = str_find_last(cp, old);
		count++;
	}
	strbuf_auto_shrink(s);

	assert_strbuf(s);
	return count;
}

void strbuf_append(strbuf *s, str t)
{
	assert_strbuf(s);
	strbuf_insert(s, t, s->size);
	assert_strbuf(s);
}

void strbuf_prepend(strbuf *s, str t)
{
	assert_strbuf(s);
	strbuf_insert(s, t, 0);
	assert_strbuf(s);
}

void strbuf_ljust(strbuf *s, char fill, isize width)
{
	assert_strbuf(s);
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

	assert_strbuf(s);
}

void strbuf_rjust(strbuf *s, char fill, isize width)
{
	assert_strbuf(s);
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

	assert_strbuf(s);
}

void strbuf_center(strbuf *s, char fill, isize width)
{
	assert_strbuf(s);
	if (s->error)
		return;

	isize sft = width - s->size;
	isize sft_l = sft / 2;
	isize sft_r = sft - sft_l;

	strbuf_ljust(s, fill, sft_l + s->size);
	strbuf_rjust(s, fill, sft_r + s->size);

	assert_strbuf(s);
}

void strbuf_reverse(strbuf *s)
{
	assert_strbuf(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size / 2; i++) {
		char tmp = s->data[i];
		s->data[i] = s->data[s->size - i - 1];
		s->data[s->size - i - 1] = tmp;
	}

	assert_strbuf(s);
}

void strbuf_to_upper(strbuf *s)
{
	assert_strbuf(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size; i++)
		s->data[i] = strlx_to_upper(s->data[i]);

	assert_strbuf(s);
}

void strbuf_to_lower(strbuf *s)
{
	assert_strbuf(s);
	if (s->error)
		return;

	for (isize i = 0; i < s->size; i++)
		s->data[i] = strlx_to_lower(s->data[i]);

	assert_strbuf(s);
}

isize strbuf_to_ll(strbuf const *s, int base, long long *num)
{
	assert_strbuf(s);
	return str_to_ll(strbuf_to_str(s), base, num);
}

void strbuf_print(strbuf const *s)
{
	assert_strbuf(s);
		if (s->error)
		return;

	for (isize i = 0; i < s->size; i++)
		putchar(s->data[i]);
}
