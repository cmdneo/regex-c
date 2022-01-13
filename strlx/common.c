#include "strlx/strlx.h"

int str_is_valid_range(isize size, isize start, isize end)
{
	return !(start > end || end > size || start < 0);
}

void strbuf_show_error(enum str_error error)
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
