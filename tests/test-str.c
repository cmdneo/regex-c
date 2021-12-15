#include <stdio.h>
#include <stdlib.h>

#include "str/str.h"

void test(int expr)
{
	static size_t counter = 0;
	counter++;

	if ((expr))
		printf("%3zu. OK\n", counter);
	else
		printf("%3zu. FAIL\n", counter);
}

strbuf readline()
{
	return strbuf_create_from_file(stdin, '\n');
}

int main()
{
	str s = cstr("hello test 1 ok test next 2 ok not now 3 ok end");
	str c1 = cstr("matches");
	str c2 = cstr("matches");
	str c3 = cstr("does not matches");
	str f = cstr("ok");

	test(f.size == 2);
	test(str_cmp(str_substr(s, 0, 5), cstr("hello")) == 0);
	test(str_cmp(str_substr(s, 13, 15), cstr("ok")) == 0);
	test(str_cmp(cstr("ok"), cstr("okok but in substr")) != 0);
	test(str_find_first(s, f) == 13);
	test(str_find_last(s, f) == 41);

	str split_me = cstr("hello:/:123:/:abc");
	str popped = str_pop_first_split(&split_me, cstr(":/:"));
	test(popped.size == 5 && split_me.size == 9 && popped.data == split_me.data - 8);

	strbuf tbc = strbuf_create_from_str(f);

	strbuf_append(&tbc, c1);
	test(str_cmp(strbuf_to_str(&tbc), cstr("okmatches")) == 0);

	strbuf_prepend(&tbc, c1);
	test(str_cmp(strbuf_to_str(&tbc), cstr("matchesokmatches")) == 0);

	strbuf_insert(&tbc, f, 7);
	test(str_cmp(strbuf_to_str(&tbc), cstr("matchesokokmatches")) == 0);

	strbuf_destroy(&tbc);

	return EXIT_SUCCESS;
}
