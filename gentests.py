r"""
Brief: A python script for generating C files for automated testing
from test-data(.tdata) files
Author: Amiy Kumar

usage: gentests.py [-h] -o OUTFILE -i INFILE [-I [INCLUDES ...]]

Code generator for testing

options:
  -h or --help                    show this help message and exit
  -o or --outfile OUTFILE
  -i or --infile INFILE
  -I or --includes [INCLUDES ...] Include files to be used in a module


Test-data(.tdata) file format:
=> <TEST NAME>
# Single line comments start with a hash-symbol(#) and are ignored
<%
# Insert test code TEMPLATE here
# Strings starting with $<arg-name> are replaced by respective values
# from arg-list. It also provides 10 tmp variables (tmp0-tmp9)
# Use temp vars like $tmpN, they are replaced by random names to prevent
# collision with with other tmp vars in the same submodule/module
# Assign the truth value of the test to the variable (int)result. Like:
result = str_cmp($s1, $expected) == 0;
%>
# Arg list (start with a colon :)
:<arg1-name>  <arg2-name> ..... <argn-name>
# Supported argument types are following C literals:
# int(dec, oct & hex) [with signs]
# float(dec and hex)  [with signs]
# char and char* (that is string)
<arg1-value>  <arg2-value> .... <argn-value>
<arg1-value>  <arg2-value> .... <argn-value>
    .           .                    .
    .           .                    .
    .           .                    .

test-data file Example:
=> reverse_digits
<%
    int $tmp0 = reverse_digits($number)
    # Set result to the truth value of the test result
    result = $tmp0 == $reversed_number;
%>
# Argument names
:number        reversed_number
# Argument value list
 12345         54321
 4242          2424
 313           313
"""

import argparse
import datetime
import functools
import pathlib
import re
import secrets
import string
import sys
from dataclasses import dataclass


TEST_START_TAG = "=>"
COMMENT_START_TAG = "#"
ARGS_START_TAG = ":"
BLOCK_START_TAG = "<%"
BLOCK_END_TAG = "%>"
# As per C11 stnadard
C_LITERAL_REGEXES = {
    "char":  (r"(L|u|U)?'([^'\\\n]|(\\[0-7]{1,3}|(\\x[\da-fA-F]+)"
              r"|\\'|\\\"|\\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v))'"),
    "string": r'(u8|u|U|L)?"([^"\\\n]|(\\.))*"',
    # Here we are matching numberic literals(ints floats) along with their
    # signs(+ or -). In C they are considered Unary operators and aren't part
    # of these numeric literals, but not here
    "float_hex": (r'[+-]?' r'(0x|0X)(([\da-fA-F]*\.[\da-fA-F]+)'
                  r'|([\da-fA-F]+\.)|([\da-fA-F]+))([pP][+-]?\d+)?[flFL]?'),
    "float_dec":  r'[+-]?' r'((\d*\.\d+)|(\d+\.)|(\d+))([eE][+-]?\d+)?[flFL]?',
    "int_hex": r'[+-]?' r'(0x|0X)[0-9a-fA-F]+(u|U|l|L|ll|LL)?',
    "int_oct": r'[+-]?' r'0[0-7]*(u|U|l|L|ll|LL)?',
    "int_dec": r'[+-]?' r'[1-9]\d*(u|U|l|L|ll|LL)?',
}
# Combine all the above regexes into a single regex by grouping and OR
C_LITERAL_REGEX = re.compile("|".join(
    [f"(?P<{cname}>{regex})" for cname, regex in C_LITERAL_REGEXES.items()]))

# C Code templates for code generation
# Main template
# NEEDS: MODULE_CODE, MODULE_NAME, INCLUDE_STATEMENTS, GENERATION_TIME
C_MODULE_TMPL = string.Template(R"""/*
 * Auto-generated by gentests.py, *Do not edit*
 * Generation time: $GENERATION_TIME
 * Generated from : $MODULE_NAME
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

$INCLUDE_STATEMENTS

static size_t grand_total = 0;
static size_t total_ok = 0;
static size_t total_fail = 0;
static size_t ntests = 0;
static size_t cases_total = 0;
static size_t cases_ok = 0;
static size_t cases_fail = 0;
static int exit_status = 0;

static void judge_test_case(int result) {
    grand_total++;
    cases_total++;

    if (result) {
        cases_ok++;
        total_ok++;
        putchar('.');
    } else {
        cases_fail++;
        total_fail++;
        exit_status = 1;
        putchar('x');
    }
    /* Add new line after 40 chars */
    if (cases_total % 40 == 0)
        putchar('\n');
}

static void init_test() {
    ntests++;
    cases_total = 0;
    cases_ok = 0;
    cases_fail = 0;
}

int main() {
    int result = 0;

    printf("======= Start '%s' Test Module =======\n", "$MODULE_NAME");

$MODULE_CODE

    printf("======= End '%s' Test Module =======\n", "$MODULE_NAME");
    printf(
        "Module summary\n"
        "Total Successes:%6zu\n"
        "Total Failures :%6zu\n"
        "Grand Total    :%6zu\n",
        total_ok, total_fail, grand_total
    );

    return exit_status;
}
""")

# Wraps several tests and shows end, start messages and submod summary
# NEEDS: TEST_CODE, TEST_NAME
C_WRAP_TESTS_TMPL = string.Template(R"""
    init_test();
    printf("=> %zu: TEST '%s'\n", ntests, "$TEST_NAME");

$TEST_CODE

    printf(
        "\nTest '%s' summary\n"
        "Pass :%4zu\n"
        "Fail :%4zu\n"
        "Total:%4zu\n"
        "--------------------------------------------------\n",
        "$TEST_NAME", cases_ok, cases_fail, cases_total
    );
""")

# For a single test entry in data file
# NEEDS: SINGLE_TEST_CODE
# SINGLE_TEST_CODE must set the integer variable 'result' to correct
# truth value indicating the success/failure of the test. For example:
# result = add(2, 4) == 6;
C_TEST_TMPL = string.Template(R"""
    result = 0;
    {
$SINGLE_TEST_CODE
    }
    judge_test_case(result);
""")


@dataclass
class Test:
    name: str
    code: str
    nargs: int
    arg_names: list[str]
    argvals: list[list[str]]


def escape(s):
    return repr(str(s).strip().replace('"', '\\"'))[1:-1]


def parse_args(line: str) -> list[str]:
    args = []

    while 1:
        match = C_LITERAL_REGEX.search(line)
        if not match:
            break
        line = line[match.end():]
        args.append(match.group())

    return args


def parse_tdf(lines: list[str], fpath: str) -> list[Test]:
    tests: list[Test] = []
    active_test: Test = None
    inside_code_block = False
    arg_line = ""

    for i, line in enumerate(lines):
        s_line = line.strip()
        # Initialize new test when TEST_START_TAG is encountered
        if s_line.startswith(TEST_START_TAG):
            test_name = escape(s_line.removeprefix(TEST_START_TAG))
            active_test = Test(test_name, "", 0, [], [])
            tests.append(active_test)
        elif s_line.startswith(COMMENT_START_TAG) or s_line == "":
            continue

        # Code extraction
        elif s_line.startswith(BLOCK_START_TAG):
            inside_code_block = True
        elif s_line.endswith(BLOCK_END_TAG):
            inside_code_block = False
        elif active_test and inside_code_block:
            active_test.code += line

        # Argument name and value list processing
        elif s_line.startswith(ARGS_START_TAG):
            active_test.arg_names = s_line.removeprefix(ARGS_START_TAG).split()
            active_test.nargs = len(active_test.arg_names)
        elif active_test and not inside_code_block:
            argvals = parse_args(s_line)
            if len(argvals) != active_test.nargs:
                raise ValueError(
                    f"Argument values list incosistent\n"
                    f"File: {fpath!r}: line {i + 1}:\n"
                    f"{line}\n"
                    f"While arg-names line is:\n"
                    f"{arg_line}\n"
                )
            active_test.argvals.append(argvals)

    return tests


def gen_mod_code(tests_mod: list[Test],
                 include_stmts: list[str], mod_name: str) -> str:
    module_code = ""
    gen_time = datetime.datetime.today().strftime("%Y-%m-%d %H:%M:%S (local)")

    for test in tests_mod:
        tests_code = ""
        for argval in test.argvals:
            # Make dict mapping from tmp-vars(tmp0-tmp9) and
            # string template variable names to their substitutions values
            tmp_var_ids = {f"tmp{x}": "tmp_" +
                           secrets.token_hex(8) for x in range(10)}
            vals_map = {
                **dict(zip(test.arg_names, argval)),
                **tmp_var_ids
            }
            tmp_code = string.Template(test.code).substitute(**vals_map)
            tests_code += C_TEST_TMPL.substitute(SINGLE_TEST_CODE=tmp_code)

        submod_tests_code = C_WRAP_TESTS_TMPL.substitute(
            TEST_CODE=tests_code,
            TEST_NAME=escape(test.name)
        )
        module_code += submod_tests_code

    return C_MODULE_TMPL.substitute(
        MODULE_CODE=module_code,
        MODULE_NAME=escape(mod_name),
        INCLUDE_STATEMENTS=include_stmts,
        GENERATION_TIME=gen_time
    )


def main():
    parser = argparse.ArgumentParser(description="Code generator for testing")
    parser.add_argument("-o", "--outfile", required=True)
    parser.add_argument("-i", "--infile", required=True)
    parser.add_argument(
        "-I", "--includes", nargs="*",
        help="Include files to be used in a module"
    )

    args = parser.parse_args()
    includes = args.includes or []
    in_fpath = pathlib.Path(args.infile)
    out_fpath = pathlib.Path(args.outfile)
    codegen_dir = out_fpath.parent

    with in_fpath.open() as f:
        tests = parse_tdf(f.readlines(), str(in_fpath))

    inc_stmts = "\n".join([f'#include "{include}"' for include in includes])
    module_code = gen_mod_code(tests, inc_stmts, in_fpath.name)

    if not codegen_dir.exists():
        print(f"Directory '{codegen_dir}' doesn't exist, creating new.")
        pathlib.Path(codegen_dir).mkdir(parents=True)
    with open(out_fpath, "w") as f:
        f.write(module_code)
        print(f"{parser.prog}: INFO: Test code generated from: "
              f"'{in_fpath}' written to: '{out_fpath}'")


if __name__ == "__main__":
    print_err = functools.partial(print, file=sys.stderr)
    try:
        main()
    except ValueError as e:
        print_err("ERROR:", e,)
        print_err("EXIT!!1")
        sys.exit(1)
