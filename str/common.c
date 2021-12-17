#include <stdio.h>

#include "str/str.h"

int strlib_is_valid_range(isize_t size, isize_t start, isize_t end)
{
	return !(start > end || end > size || start < 0);
}

void strlib_show_error_impl(int error)
{
	str err_str;

	switch (error) {
	case STRLIB_INVALID_INDEX:
		err_str = cstr("Invalid Index/Indices given!\n");
		break;

	case STRLIB_NO_MEM:
		err_str = cstr("Memory allocation failed!\n");
		break;

	default:
		break;
	}

	str_print(err_str);
}

void strlib_print_char(char c)
{
	putchar(c);
}
