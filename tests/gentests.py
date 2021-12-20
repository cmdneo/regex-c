import pathlib
import string

TEST_START_TAG = "=>"
COMMENT_START_TAG = "#"
ARGS_START_TAG = ":"
BLOCK_START_TAG = "<%"
BLOCK_END_TAG = "%>"

TDATA_FILE_EXT = ".tdata"
DIR = pathlib.Path(__file__).parent.resolve()
INCLUDE_PATH = DIR / "../include"
TDATA_FILES = [
	"str/str.h.tdata"
]

C_TEST_TEMPLATE = string.Template("""\
#include <stdio.h>
#include <stdlib.h>

$INCLUDE_STATEMENTS

int main() {
	$TEST_SEGMENTS
	return 0;
}
""")

INCLUDE_STATEMENTS = "\n".join([f'#include "{tdf.rstrip(TDATA_FILE_EXT)}"'
	for tdf in TDATA_FILES])


def parse_tdf(lines: list[str], filename: str):
	tests = []
	# name, code, args, argvals, nargs, expected
	active_test = {}
	inside_code_block = False
	code_seg = ""

	for i, line in enumerate(lines):
		line = line.strip()
		if line.startswith(TEST_START_TAG):
			active_test = {
				"name": line.lstrip(TEST_START_TAG).strip(),
				"code": "",
				"args": [],
				"argvals": [],
				"nargs": 0,
				"expected": []
			}
			tests.append(active_test)
		elif line.startswith(BLOCK_START_TAG):
			inside_code_block = True
		elif line.endswith(BLOCK_END_TAG):
			inside_code_block = False
			active_test["code"] = code_seg
		elif line.startswith(ARGS_START_TAG):
			args = line[1:].split()
			if args[-1].lower() != "expected":
				args.append("expected")
			active_test["args"] = args[:-1]
			active_test["nargs"] = len(args) - 1
		elif line.startswith(COMMENT_START_TAG):
			continue
		elif active_test and inside_code_block:
			code_seg += line + "\n"
		elif active_test and not inside_code_block:
			argvals = line.split()
			# nargs + 1 as that also includes expected value
			if len(argvals) != active_test["nargs"] + 1:
				raise ValueError(f"Argument values list incomplete\nLine no.: {i + 1}: {filename}")
			active_test["argvals"].append(argvals[:-1])
			active_test["expected"].append((argvals[-1]))
		
	return tests	

def main():
	auto_tests = {}
	for tdf in TDATA_FILES:
		tdf_path = DIR / tdf
		with tdf_path.open() as f:
			test = parse_tdf(f.readlines(), tdf)
			auto_tests[tdf.rstrip(TDATA_FILE_EXT)] = test

	for k, v in auto_tests.items():
		pass
	import pprint
	pprint.pp(auto_tests, indent=4)


main()
