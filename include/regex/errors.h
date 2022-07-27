#ifndef INCLUDE_REGEX_ERRORS_H
#define INCLUDE_REGEX_ERRORS_H

enum regex_error_code {
	REGEX_NO_ERR,

	REGEX_NO_MEM,
	REGEX_NO_TOKENS,

	REGEX_POSIX_CHAR_CLASS_OUTSIDE,
	REGEX_EXTRA_PAREN,
	REGEX_NO_CLOSING_PAREN,
	REGEX_NO_CLOSING_BRACKET,

	REGEX_ILLEGAL_CHAR,
	REGEX_ILLEGAL_ESC,
	REGEX_TRAILING_BKSLASH,

	REGEX_HEX_TOO_BIG,
	REGEX_OCT_TOO_BIG,
	REGEX_INVALID_HEX,
	REGEX_INVALID_GROUP,
	REGEX_INVALID_RANGE,
	REGEX_INVALID_DIG_SEQ,
	REGEX_INVALID_EXTENSION,
	REGEX_INVALID_CHAR_RANGE,
	REGEX_INVALID_CHAR_CLASS,
	REGEX_INVALID_POSIX_CHAR_CLASS,

	REGEX_NERRORS,
};

static const char *regex_error_messages[REGEX_NERRORS] = {
	[REGEX_NO_ERR] = "No error",
	[REGEX_NO_MEM] = "Memory allocation error",
	[REGEX_NO_TOKENS] = "No matching tokens",
	[REGEX_POSIX_CHAR_CLASS_OUTSIDE] =
		"Posix character class must be inside a character class",
	[REGEX_EXTRA_PAREN] = "Extra parenthesis",
	[REGEX_NO_CLOSING_PAREN] = "Unclosed parenthesis",
	[REGEX_NO_CLOSING_BRACKET] = "No closing bracket ]",
	[REGEX_ILLEGAL_CHAR] = "Illegal character",
	[REGEX_ILLEGAL_ESC] = "Illegal escape sequence",
	[REGEX_TRAILING_BKSLASH] = "Invalid or empty character class",
	[REGEX_HEX_TOO_BIG] = "Invalid POSIX character class",
	[REGEX_OCT_TOO_BIG] = "Hex bigger than CHAR_MAX",
	[REGEX_INVALID_HEX] = "Oct bigger than CHAR_MAX",
	[REGEX_INVALID_GROUP] = "Invalid hex",
	[REGEX_INVALID_RANGE] = "Non existent capture group number",
	[REGEX_INVALID_DIG_SEQ] =
		"Invalid digit escape sequence(group number or octal escape sequence)",
	[REGEX_INVALID_EXTENSION] = "Invalid range",
	[REGEX_INVALID_CHAR_RANGE] = "Invalid char range in character class",
	[REGEX_INVALID_CHAR_CLASS] = "Non existent extension prefix",
	[REGEX_INVALID_POSIX_CHAR_CLASS] = "Unescaped backslash",
};

static const char *regex_error(enum regex_error_code err)
{
	if (!(0 <= err && err < REGEX_NERRORS))
		return "Invalid Error code passed! No such error.";
	return regex_error_messages[err];
}

#endif
