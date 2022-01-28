#include "strlx/strlx.h"

void strlx_adjust_range(isize size, isize *start, isize *end)
{
	if (*start > *end)
		*start = *end;

	if (*start > size)
		*start = size;
	else if (*start < 0)
		*start = 0;

	if (*end > size)
		*end = size;
	else if (*end < 0)
		*end = 0;
}

int strlx_is_range_valid(isize size, isize start, isize end)
{
	return !(start > end || end > size || start < 0);
}

void strbuf_show_error(enum strlx_error error)
{
	str err_str;

	switch (error) {
	case STRLX_NO_ERR:
		err_str = M_str("");
		break;

	case STRLX_INVALID_INDEX:
		err_str = M_str("Invalid Index/Indices given!\n");
		break;

	case STRLX_NO_MEM:
		err_str = M_str("Memory allocation failed!\n");
		break;

	case STRLX_NOT_FOUND:
		err_str = M_str("Not found!\n");
		break;

	case STRLX_ALREADY_EXISTS:
		err_str = M_str("Already exists!\n");
		break;
	}

	str_print(err_str);
}
