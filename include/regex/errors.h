#ifndef INCLUDE_REGEX_ERRORS_H
#define INCLUDE_REGEX_ERRORS_H

enum regex_error {
	REGEX_NO_ERR = 0,
	REGEX_NO_MATCH,

	REGEX_POSIX_CHAR_CLASS_OUTSIDE,
	REGEX_EXTRA_PAREN,
	REGEX_NO_CLOSING_PAREN,
	REGEX_NO_CLOSING_BRACKET,

	REGEX_ILLEGAL_CHAR,
	REGEX_ILLEGAL_ESC,

	REGEX_HEX_TOO_BIG,
	REGEX_OCT_TOO_BIG,
	REGEX_INVALID_HEX,
	REGEX_INVALID_GROUP,
	REGEX_INVALID_RANGE,
	REGEX_INVALID_EXT,
	REGEX_INVALID_CHAR_RANGE,
	REGEX_INVALID_CHAR_CLASS,
	REGEX_INVALID_POSIX_CHAR_CLASS,

	REGEX_TRAILING_BKSLASH,
	REGEX_NO_MEM,
};

static char *regex_perr(enum regex_error err)
{
	char *s = "No error";

	switch (err) {
	case REGEX_NO_ERR:
		s = "No error";
		break;
	case REGEX_NO_MATCH:
		s = "No matching tokens";
		break;
	case REGEX_POSIX_CHAR_CLASS_OUTSIDE:
		s = "Posix character class must be inside a char class";
		break;
	case REGEX_EXTRA_PAREN:
		s = "Extra parenthesis";
		break;
	case REGEX_NO_CLOSING_PAREN:
		s = "Unclosed parenthesis";
		break;
	case REGEX_NO_CLOSING_BRACKET:
		s = "No closing bracket ]";
		break;
	case REGEX_ILLEGAL_CHAR:
		s = "Illegal character";
		break;
	case REGEX_ILLEGAL_ESC:
		s = "Illegal escape sequence";
		break;
	case REGEX_INVALID_CHAR_CLASS:
		s = "Invalid or empty character class";
		break;
	case REGEX_INVALID_POSIX_CHAR_CLASS:
		s = "Invalid POSIX character class";
		break;
	case REGEX_HEX_TOO_BIG:
		s = "Hex bigger than CHAR_MAX";
		break;
	case REGEX_OCT_TOO_BIG:
		s = "Oct bigger than CHAR_MAX";
		break;
	case REGEX_INVALID_HEX:
		s = "Invalid hex";
		break;
	case REGEX_INVALID_GROUP:
		s = "Non existent capture group number";
		break;
	case REGEX_INVALID_RANGE:
		s = "Invalid range";
		break;
	case REGEX_INVALID_CHAR_RANGE:
		s = "Invalid char range in character class";
		break;
	case REGEX_INVALID_EXT:
		s = "Non existent extension prefix";
		break;
	case REGEX_TRAILING_BKSLASH:
		s = "Unescaped backslash";
		break;
	case REGEX_NO_MEM:
		s = "Memory allocation error";
		break;
	}

	return s;
}

#endif
