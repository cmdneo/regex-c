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

