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
	if (!strlib_is_valid_range(s.size, start, end))
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

isize_t str_find_first(str s, str substr)
{
	isize_t ncmps = s.size - substr.size + 1;
	for (isize_t i = 0; i < ncmps; i++) {
		isize_t cmp_result = str_cmp(str_substr(s, i, i + substr.size), substr);
		if (cmp_result == 0)
			return i;
	}

	return -substr.size;
}

isize_t str_find_last(str s, str substr)
{
	isize_t ncmps = s.size - substr.size + 1;
	for (isize_t i = ncmps - 1; i >= 0; i--) {
		isize_t cmp_result = str_cmp(str_substr(s, i, i + substr.size), substr);
		if (cmp_result == 0)
			return i;
	}

	return -substr.size;
}

int str_contains(str s, str substr)
{
	return str_find_first(s, substr) >= 0;
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
		strlib_print_char(s.data[i]);
}
