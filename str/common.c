#include "str/str.h"

int str_is_valid_range(isize_t size, isize_t start, isize_t end)
{
	return !(start > end || end > size || start < 0);
}

void strbuf_show_error(int error)
{
	str err_str;

	switch (error) {
	case STR_INVALID_INDEX:
		err_str = cstr("Invalid Index/Indices given!\n");
		break;

	case STR_NO_MEM:
		err_str = cstr("Memory allocation failed!\n");
		break;

	default:
		err_str = cstr("OK, No errors.");
		break;
	}

	str_print(err_str);
}
