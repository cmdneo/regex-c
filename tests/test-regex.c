#include <stdio.h>
#include <stdlib.h>

#include "strlx/strlx.h"
#include "regex/regex.h"

int main()
{
	strbuf *s = strbuf_from("r(A|X)C{42}[[:ascii:]]");
	re_parse(s);
	strbuf_destroy(&s);
	return 0;
}
