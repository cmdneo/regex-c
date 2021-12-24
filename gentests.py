"""
Brief: A python script for generating C files for automated testing
from test-data(.tdata) files
Author: Amiy Kumar

Usage Information:

Test-data(.tdata) file format:
=> <TEST NAME>
# Single line only Comments ignored by parser
<%
# Strings starting with $<arg-name> are replaced by respective values from arg
# list Insert test code here, it also provides 10 tmp variables
# Use tmp vars like $tmp0 $tmp1, they are replaced by random names to prevent
# Collision with with other tmp vars in the same submodule/module
# After testing is done assign the truth value of the test to result(type:int) var
%>
# Arg list (start with a colon :)
:<arg1-name>  <arg2-name> ..... expected
# Supported argument types are following C literals:
# int(dec, oct & hex) [with signs]
# float(dec and hex)  [with signs]
# char and char* (that is string)
<arg1-value>  <arg2-value> .... <comparison-value>
<arg1-value>  <arg2-value> .... <comparison-value>
      .           .                    .
	  .           .                    .
	  .           .                    . 

test-data file Example:
=> STRBUF_APPEND
<%
strbuf $tmp0 = cstr($s);
strbuf_append(&$tmp0, cstr($append_this));
result = strbuf_cmp(&$tmp0, cstr($expected)) == 0;
%>
# Last arg should always be named expected
:s         append_this     expected
"hello, "     "world"      "hello, world"
"123"         "456"         "123456"


usage: python3 gentests.py [-h] [-I [INCLUDES ...]] [--tests TESTS] [--codegen CODEGEN] -o OUTFILE -i INFILE

Code generator for automated testing

options:
  -h, --help               show this help message and exit
  --tests       TESTS      Test data directory      (default=tests/)
  --codegen     CODEGEN    Codegen output directory (default=build/tests)
  -o, --outfile OUTFILE
  -i, --infile  INFILE
  -I [INCLUDES ...], --includes [INCLUDES ...]
                           Include files to be used for a module

"""

import argparse
import pathlib
import re
import string
import secrets
import sys
from dataclasses import dataclass


TEST_START_TAG = "=>"
COMMENT_START_TAG = "#"
ARGS_START_TAG = ":"
BLOCK_START_TAG = "<%"
BLOCK_END_TAG = "%>"
# As per C11 stnadard
C_LITERAL_REGEXES = {
	# Character constant
	"char": r"(L|u|U)?'([^'\\\n]|(\\[0-7]{1,3}|(\\x[\da-fA-F]+)|\\'|\\\"|\\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v))'",
	# String Literal
	"string": r'(u8|u|U|L)?"([^"\\\n]|(\\.))*"',
	# Here we are matching numbers(integers and floats) along with their signs(+ or -).
	# In C they are considered Unary operators and aren't part of these, but not here.
	# Decimal floating constant
	"float_dec": r'(\+|-)?' r'((\d*\.\d+)|(\d+\.)|(\d+))([eE](\+|-)?\d+)?[flFL]?',
	# Hexadecimal floating constant
	"float_hex": r'(\+|-)?' r'(0x|0X)(([\da-fA-F]*\.[\da-fA-F]+)|([\da-fA-F]+\.)|([\da-fA-F]+))([pP](\+|-)?\d+)?[flFL]?',
	# Integer: Decimal constant
	"int_dec": r'(\+|-)?' r'[1-9]\d*(u|U|l|L|ll|LL)?',
	# Integer: Octal constant
	"int_oct": r'(\+|-)?' r'0[0-7]*(u|U|l|L|ll|LL)?',
	# Integer: Hexadecimal constant
	"int_hex": r'(\+|-)?' r'(0x|0X)[0-9a-fA-F]+(u|U|l|L|ll|LL)?',
}
# Combine all the above regexes into a single regex by grouping and using the '|'
C_LITERAL_REGEX = re.compile("|".join([f"(?P<{cname}>{regex})"
	for cname, regex in C_LITERAL_REGEXES.items()]))

# C Code templates for code generation
# Main template
# NEEDS: MODULE_CODE, MODULE_NAME, INCLUDE_STATEMENTS
C_MODULE_TMPL = string.Template(R"""
#include <stdio.h>
#include <stdlib.h>

$INCLUDE_STATEMENTS

int main() {
	size_t grand_total = 0;
	size_t total_ok = 0;
	size_t total_fail = 0;
	size_t mod_total = 0;
	size_t mod_ok = 0;
	size_t mod_fail = 0;
	int result = 0;
	int exit_status = 0;

	printf("*****START '%s' TEST MODULE*****\n\n", "$MODULE_NAME");

$MODULE_CODE

	printf("\n\n");
	printf("*******END '%s' TEST MODULE*****\n", "$MODULE_NAME");
	printf(
		"'%s' MODULE FINAL SUMMARY\n"
		"Successes: %6zu\n"
		"Failures : %6zu\n"
		"Total    : %6zu\n",
		"$MODULE_NAME", total_ok, total_fail, grand_total
	);

	return exit_status;
}
""")

# Wraps several tests and shows end, start messages and submod summary
# NEEDS: TEST_CODE, TEST_NAME
C_WRAP_TESTS_TMPL = string.Template(R"""
	mod_total = 0;
	mod_ok = 0;
	mod_fail = 0;

	printf("-----Starting: '%s' TEST-----\n", "$TEST_NAME");

$TEST_CODE

	printf("\n");
	printf("----------End: '%s' TEST-----\n", "$TEST_NAME");
	printf(
		"Test summary\n"
		"Successes: %6zu\n"
		"Failures : %6zu\n"
		"Total    : %6zu\n",
		mod_ok, mod_fail, mod_total
	);
""")

# For a single test entry in data file
# NEEDS: SINGLE_TEST_CODE
# SINGLE_TEST_CODE must set the integer variable 'result' to correct truth value
# indicating the success/failure of the test. For example to test an add function:
# result = add(2, 4) == 6;
C_TEST_TMPL = string.Template(R"""
	grand_total++;
	mod_total++;
	result = 0;

$SINGLE_TEST_CODE

	if (result) {
		mod_ok++;
		total_ok++;
		putchar('.');
	} else {
		mod_fail++;
		total_fail++;
		exit_status = 1;
		putchar('x');
	}
	/* Add new line after 20 chars */
	if (mod_total % 20 == 0)
		putchar('\n');
""")


@dataclass
class Test:
	name: str
	code: str
	arg_names: list[str]
	argvals: list[list[str]]
	expected_vals: list[str]
	nargs: int
	expected_var = "expected"

def parse_args(line):
	args = []
	while 1:
		match = C_LITERAL_REGEX.search(line)
		if not match:
			break
		line = line[match.end():]
		args.append(match.group())
	
	return args
	

def parse_tdf(lines: list[str], fpath):
	tests: list[Test] = []
	active_test: Test = None
	inside_code_block = False
	arg_line = ""

	for i, line in enumerate(lines):
		line = line.strip()
		# Initialize new test when TEST_START_TAG is encountered
		if line.startswith(TEST_START_TAG):
			active_test = Test(
				line.lstrip(TEST_START_TAG).strip(),
				"", [], [], [], 0
			)
			tests.append(active_test)
		elif line.startswith(COMMENT_START_TAG) or line == "":
			continue
		elif line.startswith(BLOCK_START_TAG):
			inside_code_block = True
		elif line.endswith(BLOCK_END_TAG):
			inside_code_block = False
		# Extract code inside the code block <% ..... %>
		elif active_test and inside_code_block:
			active_test.code += line + "\n"

		# Extract argument names
		elif line.startswith(ARGS_START_TAG):
			arg_line = line
			args = line[1:].split()
			if args[-1].lower() != "expected":
				args.append("expected")
			# Get args from 0..last-1 as last one is the expected(non-fn-arg)
			active_test.arg_names = list(map(lambda s: s.strip(), args[:-1]))
			active_test.nargs = len(args) - 1

		# Extract argyment values
		elif active_test and not inside_code_block:
			argvals = parse_args(line)
			# nargs + 1 as that row also includes expected value
			if len(argvals) != active_test.nargs + 1:
				raise ValueError(f"""\
Argument values list incosistent
File: {fpath!r}: line {i + 1}:
{line}
While arg-names line is:
{arg_line}
""")
			active_test.argvals.append(argvals[:-1])
			active_test.expected_vals.append(argvals[-1])

	return tests	


# GENERATE FULL code from using templates for a module
def gen_mod_code(test_mod: list[Test], include_stmts, mod_name):
	module_code = ""

	for test in test_mod:
		tests_code = ""
		for argval, exp_val in zip(test.argvals, test.expected_vals):
			# Generate tmp0-tmp9 variable names
			tmp_var_ids = {f"tmp{x}": "tmp_" + secrets.token_hex(16) for x in range(10)}
			# Make dict mapping from 
			# string template variable names to their substitutions values
			vals_map = {
				test.expected_var: exp_val,
				**dict(zip(test.arg_names, argval)),
				**tmp_var_ids
			}
			tmp_code = string.Template(test.code).substitute(**vals_map)
			# Wrap the the single test code and append to tests_code
			tests_code += C_TEST_TMPL.substitute(SINGLE_TEST_CODE=tmp_code)

		# Wrap all tests of a submodule with entry and exit messages
		submod_tests_code = C_WRAP_TESTS_TMPL.substitute(
			TEST_CODE=tests_code,
			TEST_NAME=test.name
		)
		module_code += submod_tests_code

	# Put module code into main function with include statements and return
	return C_MODULE_TMPL.substitute(
		MODULE_CODE=module_code,
		MODULE_NAME=mod_name,
		INCLUDE_STATEMENTS=include_stmts
	)
		
		

def main():
	parser = argparse.ArgumentParser(description="Code generator for automated testing")
	parser.add_argument("--tests", default="tests/",help="Test data directory (default=tests/)")
	parser.add_argument("--codegen", default="build/tests/",help="Codegen output directory (default=build/tests)")
	parser.add_argument("-o", "--outfile", required=True)
	parser.add_argument("-i", "--infile", required=True)
	parser.add_argument(
		"-I", "--includes", nargs="*",
	 	help="Include files to be used for a module"
	)

	args = parser.parse_args()
	includes = args.includes or []
	tests_dir = args.tests
	codegen_dir = args.codegen
	infile = args.infile
	outfile = args.outfile
	inpath = pathlib.Path(tests_dir) / infile
	outpath = pathlib.Path(codegen_dir) / outfile

	with inpath.open() as f:
		tests = parse_tdf(f.readlines(), infile)

	inc_stmts = "\n".join([f'#include "{include}"' for include in includes])
	module_code = gen_mod_code(tests, inc_stmts, infile)

	# Write code the file and print message
	if not pathlib.Path(codegen_dir).exists():
		print(f"Directory '{codegen_dir}' doesn't exist, creating new.")
		pathlib.Path(codegen_dir).mkdir(parents=True)
	with open(outpath, "w") as f:
		f.write(module_code)
		print(f"{parser.prog}: [INFO] Test code generated from: '{infile}' written to: '{outfile}' at '{codegen_dir}'")

if __name__ == "__main__":
	try:
		main()
	except ValueError as e:
		print("ERROR:", e)
		print("EXIT!!1")
		sys.exit(1)