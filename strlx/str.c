#include <inttypes.h>
#include <assert.h>

#include "strlx/strlx.h"

static const char DIG_VAL[] = {
	['0'] = 0,  ['1'] = 1,	['2'] = 2,  ['3'] = 3,	['4'] = 4,  ['5'] = 5,
	['6'] = 6,  ['7'] = 7,	['8'] = 8,  ['9'] = 9,	['A'] = 10, ['B'] = 11,
	['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15, ['G'] = 16, ['H'] = 17,
	['I'] = 18, ['J'] = 19, ['K'] = 20, ['L'] = 21, ['M'] = 22, ['N'] = 23,
	['O'] = 24, ['P'] = 25, ['Q'] = 26, ['R'] = 27, ['S'] = 28, ['T'] = 29,
	['U'] = 30, ['V'] = 31, ['W'] = 32, ['X'] = 33, ['Y'] = 34, ['Z'] = 35,
};
static const str STR_DIGS_36 = M_str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

str cstr(char const *s)
{
	isize i;
	for (i = 0; s[i] != '\0'; i++)
		;
	return (str){ .size = i, .data = s };
}

str str_substr(str s, isize start, isize end)
{
	strlx_adjust_range(s.size, &start, &end);

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

isize str_cmp_case(str s, str t)
{
	isize min_size = s.size < t.size ? s.size : t.size;
	isize ret = 0;

	for (isize i = 0; i < min_size; i++) {
		ret = strlx_to_lower(s.data[i]) - strlx_to_lower(t.data[i]);
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

int str_has_char_case(str s, char c)
{
	c = strlx_to_lower(c);
	for (isize i = 0; i < s.size; i++)
		if (strlx_to_lower(s.data[i]) == c)
			return 1;

	return 0;
}

int str_starts_with(str s, str t)
{
	if (t.size > s.size)
		return 0;
	return str_cmp(str_substr(s, 0, t.size), t) == 0;
}

int str_starts_with_case(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp_case(str_substr(s, 0, t.size), t) == 0;
}

int str_ends_with(str s, str t)
{
	if (t.size > s.size)
		return 0;
	return str_cmp(str_substr(s, s.size - t.size, s.size), t) == 0;
}

int str_ends_with_case(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp_case(str_substr(s, s.size - t.size, s.size), t) == 0;
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

isize str_to_ll(str s, int base, long long *num)
{
	assert(num);
	assert(2 <= base && base <= 36);

	static const str bin_pref = M_str("0b");
	static const str hex_pref = M_str("0x");

	*num = 0;
	long long mul = 1;
	isize start = s.size;
	isize end = 0;
	str digs = str_substr(STR_DIGS_36, 0, base);

	for (isize i = 0; i < s.size; i++) {
		if (!strlx_is_space(s.data[i])) {
			start = i;
			break;
		}
	}

	/* String is only whitespaces */
	if (start == s.size)
		return 0;

	if (s.data[start] == '-') {
		start++;
		mul = -1;
	} else if (s.data[start] == '+')
		start++;
	/* If only sign is present => INVALID */
	if (start == s.size)
		return start - 1;

	if (base == 2 &&
	    str_starts_with_case(str_substr(s, start, s.size), bin_pref))
		start += 2;
	else if (base == 16 &&
		 str_starts_with_case(str_substr(s, start, s.size), hex_pref))
		start += 2;

	for (end = start; end < s.size; end++) {
		if (!str_has_char_case(digs, s.data[end]))
			break;
	}

	/* If no digits after SIGN or after SIGN AND PREFIX => INVALID */
	if (end == start)
		return start - 1;

	for (isize i = end - 1; i >= start; i--) {
		*num += mul * DIG_VAL[(int)strlx_to_upper(s.data[i])];
		mul *= base;
	}

	return end;
}

void str_print(str s)
{
	for (isize i = 0; i < s.size; i++)
		putchar(s.data[i]);
}
