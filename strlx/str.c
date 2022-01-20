#include <assert.h>

#include "strlx/strlx.h"

str cstr(char const *s)
{
	isize i;
	for (i = 0; s[i] != '\0'; i++)
		;
	return (str){ .size = i, .data = s };
}

str str_substr(str s, isize start, isize end)
{
	if (!strlx_is_valid_range(s.size, start, end))
		return (str){ 0 };

	return (str){
		.size = end - start,
		.data = s.data + start,
	};
}

isize str_cmp(str s, str t)
{
	isize min_size = s.size < t.size ? s.size : t.size;
	isize ret = 0;

	for (isize i = 0; i < min_size; i++) {
		ret = s.data[i] - t.data[i];
		if (ret != 0)
			break;
	}

	if (s.size > t.size)
		ret = s.data[0];
	else if (s.size < t.size)
		ret = -t.data[0];

	return ret;
}

isize str_find_first(str s, str t)
{
	if (t.size > s.size)
		return -1;

	isize ncmps = s.size - t.size + 1;
	for (isize i = 0; i < ncmps; i++) {
		isize cmp_result = str_cmp(str_substr(s, i, i + t.size), t);
		if (cmp_result == 0)
			return i;
	}

	return -1;
}

isize str_find_last(str s, str t)
{
	if (t.size > s.size)
		return -1;

	isize ncmps = s.size - t.size + 1;
	for (isize i = ncmps - 1; i >= 0; i--) {
		isize cmp_result = str_cmp(str_substr(s, i, i + t.size), t);
		if (cmp_result == 0)
			return i;
	}

	return -1;
}

isize str_count(str s, str t)
{
	if (t.size > s.size)
		return -1;

	isize count = 0;
	isize t_at = str_find_first(s, t);
	while (t_at >= 0) {
		count++;
		s = str_substr(s, t_at + t.size, s.size);
		t_at = str_find_first(s, t);
	}

	return count;
}

int str_has_char(str s, char c)
{
	for (isize i = 0; i < s.size; i++)
		if (s.data[i] == c)
			return 1;

	return 0;
}

int str_starts_with(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp(str_substr(s, 0, t.size), t) == 0;
}

int str_ends_with(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp(str_substr(s, s.size - t.size, s.size), t) == 0;
}

str str_lstrip(str s, str t)
{
	if (t.size == 0 || t.size > s.size)
		return s;

	str ret = s;
	while (str_starts_with(ret, t))
		ret = str_substr(ret, t.size, ret.size);

	return ret;
}

str str_rstrip(str s, str t)
{
	if (t.size == 0 || t.size > s.size)
		return s;

	str ret = s;
	while (str_ends_with(ret, t))
		ret = str_substr(ret, 0, ret.size - t.size);

	return ret;
}

str str_strip(str s, str t)
{
	if (t.size == 0 || t.size > s.size)
		return s;

	str ret = s;
	ret = str_lstrip(ret, t);
	ret = str_rstrip(ret, t);

	return ret;
}

str str_pop_first_split(str *s, str split_by)
{
	assert(s);

	isize first_at = str_find_first(*s, split_by);
	str ret = { .data = s->data, .size = first_at };
	/* Return full string and set s to empty string if seperator NOT found */
	if (first_at < 0) {
		ret.size = s->size;
		s->size = 0;
		return ret;
	}

	s->data = s->data + first_at + split_by.size;
	s->size -= first_at + split_by.size;

	return ret;
}

void str_print(str s)
{
	for (isize i = 0; i < s.size; i++)
		putchar(s.data[i]);
}
