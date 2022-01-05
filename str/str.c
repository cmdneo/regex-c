#include "str/str.h"

str cstr(char const *s)
{
	isize_t i;
	for (i = 0; s[i] != '\0'; i++)
		;
	return (str){ .size = i, .data = s };
}

str str_substr(str s, isize_t start, isize_t end)
{
	if (!str_is_valid_range(s.size, start, end))
		return (str){ 0 };

	return (str){
		.size = end - start,
		.data = s.data + start,
	};
}

isize_t str_cmp(str s, str t)
{
	isize_t min_size = s.size < t.size ? s.size : t.size;
	isize_t ret = 0;

	for (isize_t i = 0; i < min_size; i++) {
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

isize_t str_find_first(str s, str t)
{
	isize_t ncmps = s.size - t.size + 1;
	for (isize_t i = 0; i < ncmps; i++) {
		isize_t cmp_result = str_cmp(str_substr(s, i, i + t.size), t);
		if (cmp_result == 0)
			return i;
	}

	return -t.size;
}

isize_t str_find_last(str s, str t)
{
	isize_t ncmps = s.size - t.size + 1;
	for (isize_t i = ncmps - 1; i >= 0; i--) {
		isize_t cmp_result = str_cmp(str_substr(s, i, i + t.size), t);
		if (cmp_result == 0)
			return i;
	}

	return -t.size;
}

isize_t str_count(str s, str t)
{
	isize_t count = 0;
	isize_t t_at = str_find_first(s, t);
	while (t_at >= 0) {
		count++;
		s = str_substr(s, t_at + t.size, s.size);
		t_at = str_find_first(s, t);
	}

	return count;
}

str str_lstrip(str s, str t)
{
	str ret = s;
	isize_t substr_at = str_find_first(ret, t);
	/* Continue as long as t is at the start of the string ret and remove it */
	while (substr_at == 0) {
		ret = str_substr(ret, t.size, ret.size);
		substr_at = str_find_first(ret, t);
	}

	return ret;
}

str str_rstrip(str s, str t)
{
	str ret = s;
	isize_t substr_at = str_find_last(ret, t);
	/* Continue as long as t is at the end of the string ret and remove it */
	while (substr_at == ret.size - t.size) {
		ret = str_substr(ret, 0, substr_at);
		substr_at = str_find_last(ret, t);
	}

	return ret;
}

str str_strip(str s, str t)
{
	str ret = s;
	ret = str_lstrip(ret, t);
	ret = str_rstrip(ret, t);

	return ret;
}

str str_pop_first_split(str *s, str split_by)
{
	isize_t first_at = str_find_first(*s, split_by);
	/* Return string with size=0 if seperator NOT found */
	if (first_at < 0)
		return (str){ .data = s->data, .size = 0 };

	str ret = { .data = s->data, .size = first_at };

	s->data = s->data + first_at + split_by.size;
	s->size -= first_at + split_by.size;

	return ret;
}

void str_print(str s)
{
	for (isize_t i = 0; i < s.size; i++)
		putchar(s.data[i]);
}
