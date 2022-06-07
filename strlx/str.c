#include <assert.h>
#include <stdbool.h>

#include "strlx/strlx.h"

static const char DIG_VALS[] = {
	['0'] = 0,  ['1'] = 1,	['2'] = 2,  ['3'] = 3,	['4'] = 4,  ['5'] = 5,
	['6'] = 6,  ['7'] = 7,	['8'] = 8,  ['9'] = 9,	['A'] = 10, ['B'] = 11,
	['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15, ['G'] = 16, ['H'] = 17,
	['I'] = 18, ['J'] = 19, ['K'] = 20, ['L'] = 21, ['M'] = 22, ['N'] = 23,
	['O'] = 24, ['P'] = 25, ['Q'] = 26, ['R'] = 27, ['S'] = 28, ['T'] = 29,
	['U'] = 30, ['V'] = 31, ['W'] = 32, ['X'] = 33, ['Y'] = 34, ['Z'] = 35,
};
static const str STR_DIGS_B36 = M_str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

static inline isize min(isize a, isize b)
{
	return a < b ? a : b;
}

static inline bool str_has_any_at(str s, str chars, isize at)
{
	assert(0 <= at && at < s.size);
	for (isize i = 0; i < chars.size; i++) {
		if (chars.data[i] == s.data[at])
			return true;
	}

	return false;
}

str cstr(char const *s)
{
	isize i;
	for (i = 0; s[i] != '\0'; i++)
		;
	return (str){ .size = i, .data = s };
}

str str_substr(str s, isize start, isize end)
{
	if (!strlx_is_range_valid(s.size, start, end))
		strlx_adjust_range(s.size, &start, &end);

	return (str){
		.size = end - start,
		.data = s.data + start,
	};
}

isize str_cmp(str s, str t)
{
	isize min_size = min(s.size, t.size);
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
	isize min_size = min(s.size, t.size);
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

isize str_cmp_rev(str s, str t)
{
	isize ret = 0;
	isize size = s.size < t.size ? s.size : t.size;

	for (isize i = 0; i < size; i++) {
		if (s.data[s.size - i - 1] == t.data[i])
			ret++;
		else
			break;
	}

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

bool str_has_char(str s, char c)
{
	for (isize i = 0; i < s.size; i++)
		if (s.data[i] == c)
			return 1;

	return 0;
}

bool str_has_char_case(str s, char c)
{
	c = strlx_to_lower(c);
	for (isize i = 0; i < s.size; i++)
		if (strlx_to_lower(s.data[i]) == c)
			return 1;

	return 0;
}

bool str_starts_with(str s, str t)
{
	if (t.size > s.size)
		return 0;
	return str_cmp(str_substr(s, 0, t.size), t) == 0;
}

bool str_starts_with_case(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp_case(str_substr(s, 0, t.size), t) == 0;
}

bool str_ends_with(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp(str_substr(s, s.size - t.size, s.size), t) == 0;
}

bool str_ends_with_case(str s, str t)
{
	if (t.size > s.size)
		return 0;

	return str_cmp_case(str_substr(s, s.size - t.size, s.size), t) == 0;
}

str str_remove_prefix(str s, str pref)
{
	if (str_starts_with(s, pref))
		return str_substr(s, pref.size, s.size);

	return s;
}

str str_remove_suffix(str s, str suff)
{
	if (str_ends_with(s, suff))
		return str_substr(s, 0, s.size - suff.size);

	return s;
}

str str_lstrip(str s, str chars)
{
	while (s.size != 0 && str_has_any_at(s, chars, 0)) {
		s.data++;
		s.size--;
	}

	return s;
}

str str_rstrip(str s, str chars)
{
	while (s.size != 0 && str_has_any_at(s, chars, s.size - 1))
		s.size--;

	return s;
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

	*num = 0;
	long long mul = 1;
	isize start = s.size;
	isize end = 0;
	str digs = str_substr(STR_DIGS_B36, 0, base);

	for (isize i = 0; i < s.size; i++) {
		if (!strlx_is_space(s.data[i])) {
			start = i;
			break;
		}
	}

	/* String is only whitespaces or empty */
	if (start == s.size)
		return 0;

	if (s.data[start] == '-') {
		start++;
		mul = -1;
	} else if (s.data[start] == '+')
		start++;

	if (base == 16 && str_starts_with_case(str_substr(s, start, s.size),
					       (str)M_str("0x"))) {
		start += 2;
		/* If no valid digit just after the prefix then x|X of the prefix is
		   an invalid char and 0 of the prefix is assumed to be a digit */
		if (start == s.size || !str_has_char(digs, s.data[start]))
			return start - 1;
	}

	for (end = start; end < s.size; end++) {
		if (!str_has_char_case(digs, s.data[end]))
			break;
	}

	/* If no digits after SIGN => INVALID */
	if (end == start)
		return 0;

	for (isize i = end - 1; i >= start; i--) {
		*num += mul * DIG_VALS[(int)strlx_to_upper(s.data[i])];
		mul *= base;
	}

	return end;
}

void str_print(str s)
{
	for (isize i = 0; i < s.size; i++)
		putchar(s.data[i]);
}
