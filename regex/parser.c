#include <assert.h>
#include <limits.h>
#include <stdbool.h>

#include "strlx/strlx.h"
#include "regex/errors.h"

#include "mem.h"
#include "tokens.h"
#include "parser.h"

#define DEBUG(...) fprintf(stderr, __VA_ARGS__)

static inline int token_cmp(token_T const *a, token_T const *b)
{
	return a->type == b->type && a->value == b->value;
}

/**
 * @brief Search range [self->at...end), end < 0 => end = self->ntokens
 * 
 * @param self 
 * @param tok 
 * @param end when to stop search, negative for full
 * @return int Position of the matching token in pattern, -1 if not found
 */
static int pstate_token_pos(parser_T const *self, token_T const *tok, int end)
{
	assert(self);
	if (end < 0 || end > self->ntokens)
		end = self->ntokens;

	for (int i = self->at; i < end; i++)
		if (token_cmp(&self->tokens[i], tok))
			return self->tokens[i].pos;

	return -1;
}

/**
 * @brief Search range [self->at...end), end < 0 => end = self->ntokens
 * 
 * @param self 
 * @param tok 
 * @param end when to stop search, negative for full
 * @return int Index of the matching token in self->tokens, -1 if not found
 */
static int pstate_token_index(parser_T const *self, token_T const *tok, int end)
{
	assert(self);
	if (end < 0 || end > self->ntokens)
		end = self->ntokens;

	for (int i = self->at; i < end; i++)
		if (self->tokens[i].type == tok->type &&
		    self->tokens[i].value == tok->value)
			return i;

	return -1;
}

static egraph_T *egraph_create()
{
	egraph_T *ret = ALLOC(ret);
	if (ret == NULL)
		return NULL;

	return ret;
}

static void egraph_destroy(egraph_T **eg)
{
	assert(eg);
	assert(*eg);
	egraph_T *tmp = *eg;

	for (int i = 0; i < tmp->nnodes; i++) {
		egraph_T *tmp_node = &tmp->nodes[i];
		egraph_destroy(&tmp_node);
	}

	if (tmp->is_cclass)
		strbuf_destroy(&tmp->cclass_chars);
	if (tmp->nodes != NULL)
		FREE(tmp->nodes);

	FREE(tmp);
	*eg = NULL;
}

/**
 * @brief Inserts a new node by shallow copying node
 * 
 * @param eg
 * @param node shallow copied
 * @return egraph_T pointer to the newly inserted node
 */
static egraph_T *egraph_insert(egraph_T *eg, egraph_T *node)
{
	assert(eg);
	assert(node);

	if (eg->nnodes >= eg->nodecap) {
		int newcap = (newcap == 0) ? 1 : (eg->nodecap * 2);
		egraph_T *tmp = N_REALLOC(eg->nodes, newcap);
		if (tmp == NULL) {
			FREE(eg->nodes);
			eg->nodes = NULL;
			return NULL;
		}
		eg->nodes = tmp;
		eg->nodecap = newcap;
	}

	eg->nodes[eg->nnodes] = (egraph_T){ 0 };
	egraph_T *new_node = &eg->nodes[eg->nnodes];
	*new_node = *node;
	new_node->prev = eg;
	eg->nnodes++;

	return new_node;
}

static void egraph_debug(egraph_T *eg, int depth)
{
	for (int i = 0; i < depth - 1; i++)
		DEBUG("    ");
	if (depth > 0) {
		DEBUG("prev %p\n", (void *)(eg->prev));
		if (eg->prev && eg->prev->nnodes > 0 &&
		    &eg->prev->nodes[eg->prev->nnodes - 1] == eg)
			DEBUG(u8"└───");
		else
			DEBUG(u8"├───");
	}

	DEBUG("[%d-%d]%c ", eg->min, eg->max, (eg->lazy ? '?' : '>'));
	if (eg->is_group)
		DEBUG("()");
	else if (!eg->anychar)
		DEBUG("%c", eg->value);
	DEBUG("\n");

	for (int i = 0; i < eg->nnodes; i++)
		egraph_debug(&eg->nodes[i], depth + 1);
}

static parser_T *pstate_create(strbuf const *pattern)
{
	assert(pattern);

	parser_T *ret = ALLOC(ret);
	if (ret == NULL) {
		return NULL;
	}
	token_T *tokens = N_ALLOC(tokens, pattern->size);
	if (tokens == NULL) {
		FREE(ret);
		return NULL;
	}
	*ret = (parser_T){
		.tokens = tokens,
		.pattern = pattern,
	};

	return ret;
}

static void pstate_destroy(parser_T **self)
{
	assert(self);
	parser_T *tmp = *self;
	assert(tmp);
	assert(tmp->tokens);

	FREE(tmp->tokens);
	FREE(*self);
}

/**
 * @brief Parses two character hex sequence
 *	Format: HH, (H=hex-dig) 
 *
 * @param self 
 * @param tok 
 * @return int Error code (success=0)
 */
static int parse_hex_seq(parser_T *self, token_T *tok)
{
	assert(self);
	assert(tok);

	int const NDIGITS = 2;
	long long num = 0;
	str hexs = strbuf_substr(self->pattern, self->at, self->at + NDIGITS);

	if (hexs.size > 0 && !str_has_char(STR_HEX_DIGITS, hexs.data[0]))
		return REGEX_INVALID_HEX;
	if (str_to_ll(hexs, 16, &num) != NDIGITS)
		return REGEX_INVALID_HEX;
	if (num > CHAR_MAX)
		return REGEX_HEX_TOO_BIG;

	tok->type = RE_TC_ORD;
	tok->value = num;
	tok->chars = strbuf_substr(self->pattern, self->at - tok->chars.size,
				   self->at + NDIGITS);
	self->at += NDIGITS;
	return 0;
}

/**
 * @brief Parses number esc-seq, which can be: group-number or octal-number
 * As decided by the Format:
 * ooo | 0o | 0     => Octal number, (o=octal-digit)
 * 1-99 (inclusive) => Group number
 * 
 * @param self
 * @param tok
 * @return int Error code
 */
static int parse_dig_seq(parser_T *self, token_T *tok)
{
	assert(self);
	assert(tok);

	long long dec = 0;
	long long oct = 0;
	strbuf const *pat = self->pattern;
	int tok_start = self->at - tok->chars.size;

	// Parse max 3 chars for octal and max 2 chars for decimal.
	// If there are 3 octal digits or first digit is 0 then octal mode,
	// otherwise decimal mode(for caputure group number)
	int nocts =
		str_to_ll(strbuf_substr(pat, self->at, self->at + 3), 8, &oct);
	int ndecs =
		str_to_ll(strbuf_substr(pat, self->at, self->at + 2), 10, &dec);

	if (ndecs == 0 || ndecs == 0)
		return REGEX_ILLEGAL_ESC;

	if (pat->data[self->at] == '0' || nocts == 3) {
		if (oct > CHAR_MAX)
			return REGEX_OCT_TOO_BIG;
		tok->type = RE_TC_ORD;
		tok->value = oct;
		tok->chars = strbuf_substr(pat, tok_start, self->at + nocts);
		self->at += nocts;
	} else {
		if (!(1 <= dec && dec <= 99))
			return REGEX_INVALID_GROUP;
		tok->type = RE_TC_GNUM;
		tok->value = dec;
		tok->chars = strbuf_substr(pat, tok_start, self->at + ndecs);
		self->at += ndecs;
	}

	return 0;
}

/**
 * @brief Generates the next token and pushes it to self->tokens
 *
 * @param self
 * @return int
 */
static int parse_gen_next_token(parser_T *self)
{
	assert(self);

	const strbuf *pat = self->pattern;
	int error = REGEX_NO_TOKENS;
	int old_at = self->at;
	token_T *tok = &self->tokens[self->ntokens];
	str slice = strbuf_substr(pat, self->at, pat->size);

	// Match listed tokens(might be partial)
	for (int j = 0; j < RE_NTOKENS; j++) {
		if (!str_starts_with(slice, RE_TOKENS[j].chars))
			continue;

		*tok = RE_TOKENS[j];
		tok->pos = self->at;
		self->at += tok->chars.size;
		error = 0;
		break;
	}

	// Parse hex esc-seq digits
	if (tok->type == RE_TC_ESC_HEX)
		error = parse_hex_seq(self, tok);
	else if (tok->type == RE_TC_BSLASH) {
		// Parse digit esc-seq (like \1, \024)
		if (self->at < pat->size &&
		    str_has_char(STR_DIGITS, pat->data[self->at]))
			error = parse_dig_seq(self, tok);
		// Trailing backslash or no valid character after the backslash
		else
			error = REGEX_ILLEGAL_ESC;
	}
	// Other ordinary ASCII character
	else if (error == REGEX_NO_TOKENS && slice.size > 0) {
		*tok = (token_T){
			.value = slice.data[0],
			.type = RE_TC_ORD,
			.chars = str_substr(slice, 0, 1),
			.pos = self->at,
		};
		self->at++;
		error = 0;
	}

	if (error)
		self->at = old_at;
	else
		self->ntokens++;

	return error;
}

static int parse_gen_tokens(parser_T *self)
{
	assert(self);

	bool in_cclass = false;

	while (self->at < self->pattern->size) {
		int error = parse_gen_next_token(self);
		if (error)
			return error;
		token_T *last = &self->tokens[self->ntokens - 1];

		if (last->type == RE_TC_LBRACKET)
			in_cclass = true;
		else if (last->type == RE_TC_RBRACKET)
			in_cclass = false;

		if (!in_cclass)
			continue;
		// If inside [...] then all metachars(except ']') are oridnary
		switch (last->type) {
		case RE_TC_LBRACKET:
		case RE_TC_LBRACE:
		case RE_TC_RBRACE:
		case RE_TC_LPAREN:
		case RE_TC_RPAREN:
		case RE_TC_CARET:
		case RE_TC_DOLLAR:
		case RE_TC_QMARK:
		case RE_TC_PLUS:
		case RE_TC_ASTERISK:
		case RE_TC_PERIOD:
		case RE_TC_BAR:
			last->type = RE_TC_ORD;
			break;

		case RE_TC_ANC_BOUND_WORD:;
			int tmp = last->pos;
			*last = RE_TOKENS[RE_TC_ESC_BS];
			last->pos = tmp;
			break;

		default:
			break;
		}
	}

	self->at = 0;
	assert(self->ntokens <= self->pattern->size);

	return 0;
}

// TODO
/**
 * @brief Parses character class and adds the chars to the node
 * @param self 
 * @param node 
 * @return int 
 */
static int parse_char_class(parser_T *self, egraph_T *node)
{
	assert(self);
	assert(node);
	assert(self->tokens[self->at].type == RE_TC_LBRACKET);

	bool inverted = false;
	token_T *tok = &self->tokens[self->at];

	// Chnage(as needed) and verify token type
	switch (tok->type) {
	case RE_TC_CC_NON_DIGIT:
		inverted = true;
		/* Fallthrough */
	case RE_TC_CC_DIGIT:
		tok->type = RE_TC_PCC_DIGIT;
		break;

	case RE_TC_CC_NON_WHITESPACE:
		inverted = true;
		/* Fallthrough */
	case RE_TC_CC_WHITESPACE:
		tok->type = RE_TC_PCC_SPACE;
		break;

	case RE_TC_CC_NON_WORD_CHAR:
		inverted = true;
		/* Fallthrough */
	case RE_TC_CC_WORD_CHAR:
		tok->type = RE_TC_PCC_WORD;
		break;

	case RE_TC_PCC_ALNUM:
	case RE_TC_PCC_ALPHA:
	case RE_TC_PCC_ASCII:
	case RE_TC_PCC_BLANK:
	case RE_TC_PCC_CNTRL:
	case RE_TC_PCC_DIGIT:
	case RE_TC_PCC_GRAPH:
	case RE_TC_PCC_LOWER:
	case RE_TC_PCC_PRINT:
	case RE_TC_PCC_PUNCT:
	case RE_TC_PCC_SPACE:
	case RE_TC_PCC_UPPER:
	case RE_TC_PCC_WORD:
	case RE_TC_PCC_XDIGIT:
		break; /* Nothing */

	default:
		assert(!"Invalid token type, non word-class type token");
		break;
	}

	node->is_cclass = true;
	node->is_cclass_inv = inverted;

	return 0;
}

/**
 * @brief Parses range values inside braces{}
 * Format: {N} or {M,N} or {N,} => (N,M = +ve integers, base=10)
 * If not in this format then it({) will be converted to RE_TC_ORD type
 * 
 * @param self 
 * @return int 
 */
static int parse_braces(parser_T *self, egraph_T *node)
{
	assert(self);
	assert(node);
	assert(self->tokens[self->at].type == RE_TC_LBRACE);

	// Index of metachar '}' in self->tokens
	int end_idx = pstate_token_index(self, &RE_TOKENS[RE_TC_RBRACE], -1);
	// No closing brace or empty {}
	if (end_idx == -1 || end_idx == self->at + 1) {
		self->tokens[self->at].type = RE_TC_ORD;
		return 0;
	}

	long long min = 0;
	long long max = 0;
	int start = self->tokens[self->at].pos + 1;
	int end = self->tokens[end_idx].pos;
	int sep = pstate_token_pos(
		self, &(token_T){ .value = ',', .type = RE_TC_ORD }, end_idx);

	// Verify format
	for (int i = start; i < end; i++) {
		if (i == sep)
			continue;
		if (!str_has_char(STR_DIGITS, self->pattern->data[i])) {
			self->tokens[self->at].type = RE_TC_ORD;
			return 0;
		}
	}

	// Format {N}
	if (sep == -1) {
		str_to_ll(strbuf_substr(self->pattern, start, end), 10, &min);
		max = min;
	}
	// Format {N,}
	else if (sep == end - 1) {
		str_to_ll(strbuf_substr(self->pattern, start, sep), 10, &min);
		max = INT_MAX;
	}
	// Format {M,N}
	else {
		str_to_ll(strbuf_substr(self->pattern, start, sep), 10, &min);
		str_to_ll(strbuf_substr(self->pattern, sep + 1, end), 10, &max);

		if (min > max)
			return REGEX_INVALID_RANGE;
	}

	assert(0 <= min && min <= INT_MAX);
	assert(0 <= max && max <= INT_MAX);
	self->at = end_idx + 1;
	node->min = min;
	node->max = max;

	return 0;
}

// TODO
/**
 * @brief Parses [.......]
 *
 * Parses following things inside []:
 * Char ranges(R): <char-1> - <char-2> (inclusive), where <char-1> <= <char-2>
 * Char classes(C): \d, \D, \w, \W, \s, \S
 * and Posix char classes(P)
 * 
 * @param self 
 * @return int
 */
static int parse_brackets(parser_T *self)
{
	assert(self);
	assert(self->tokens[self->at].type == RE_TC_LBRACKET);

	int end_idx = pstate_token_index(self, &RE_TOKENS[RE_TC_RBRACKET], -1);
	if (end_idx == -1)
		return REGEX_NO_CLOSING_BRACKET;
	if (end_idx == self->at + 1)
		return REGEX_INVALID_CHAR_CLASS;

	int inverted = 0;

	if (token_cmp(&self->tokens[self->at + 1],
		      &TOKEN_NEW(RE_TC_ORD, '^'))) {
	}

	return 0;
}

static egraph_T *parse_gen_exec_graph(parser_T *self, egraph_T *parent)
{
	assert(self);
	assert(parent);

	int err = 0;
	int nmods = 1;
	egraph_T node = EMPTY_NODE;
	egraph_T *prev = parent;

	parent->is_group = 1;

	while (self->at < self->ntokens) {
		token_T *tok = &self->tokens[self->at];

		if (tok->type == RE_TC_ORD || tok->type == RE_TC_PERIOD) {
			if (tok->type == RE_TC_PERIOD)
				node.anychar = 1;
			else
				node.value = tok->value;

			node.min = 1;
			node.max = 1;
			prev = egraph_insert(prev, &node);
			node = EMPTY_NODE;
			nmods = 0;

			self->at++;
			continue;
		}

		// Do common updates at once
		switch (tok->type) {
		case RE_TC_ASTERISK_LAZY:
		case RE_TC_PLUS_LAZY:
		case RE_TC_QMARK_LAZY:
			prev->lazy = 1;
			/* Fallthrough */
		case RE_TC_ASTERISK:
		case RE_TC_PLUS:
		case RE_TC_QMARK:
			nmods++;
			/* Fallthrough */
		case RE_TC_LPAREN:
		case RE_TC_RPAREN:
		case RE_TC_BAR:
			self->at++;
			break;
		}

		switch (tok->type) {
		case RE_TC_ASTERISK_LAZY:
		case RE_TC_ASTERISK:
			prev->min = 0;
			prev->max = INT_MAX;
			break;

		case RE_TC_PLUS_LAZY:
		case RE_TC_PLUS:
			prev->min = 1;
			prev->max = INT_MAX;
			break;

		case RE_TC_QMARK_LAZY:
		case RE_TC_QMARK:
			prev->min = 0;
			prev->max = 1;
			break;

		case RE_TC_LBRACE:
			if ((err = parse_braces(self, prev)) != 0) {
				self->error = err;
				return NULL;
			}
			// If no matching format
			// Then parse that '{' again but as RE_TT_ORD(as set)
			if (tok->type == RE_TC_ORD) {
				self->at--;
				break;
			} else {
				nmods++;
			}
			// If like {...}? then lazy
			if (self->at < self->ntokens &&
			    self->tokens[self->at].type == RE_TC_QMARK) {
				prev->lazy = 1;
				self->at++;
			}
			break;

		case RE_TC_RBRACE:
			tok->type = RE_TC_ORD;
			self->at--;
			break;

		case RE_TC_LPAREN:
			// Insert an empty node for group
			prev = egraph_insert(prev, &EMPTY_NODE);
			DEBUG("%p\n", (void *)prev->prev);
			prev = parse_gen_exec_graph(self, prev);
			break;

		case RE_TC_RPAREN:
			return parent;
			break;

		case RE_TC_BAR:
			prev = parent;
			nmods = 1;
			break;

		default:
			break;
		}

		if (self->at == self->ntokens)
			break;
		// If more than one consecutive pattern modifiers applied
		// Or if modifier applied to incomplete node
		if (nmods > 1) {
			self->error = REGEX_ILLEGAL_CHAR;
			return NULL;
		}
	}

	return parent;
}

egraph_T *re_parse(strbuf const *pattern)
{
	assert(pattern);

	int error = 0;

	parser_T *self = pstate_create(pattern);
	if (self == NULL)
		return NULL;

	egraph_T *ret = egraph_create();
	if (ret == NULL) {
		pstate_destroy(&self);
		return NULL;
	}
	self->exec_graph = ret;

	error = parse_gen_tokens(self);
	if (error) {
		self->error = error;
		goto err_return;
	}

#ifndef NDEBUG
	printf("POS VAL TYP TOKEN\n");
	for (int i = 0; i < self->ntokens; i++) {
		token_T tok = self->tokens[i];
		printf("%3d %3d %3d \x1b[1;31m=>\x1b[0m", tok.pos, tok.value,
		       tok.type);
		str_print(tok.chars);
		printf("\x1b[1;31m<=\x1b[0m\n");
	}
#endif

	if (parse_gen_exec_graph(self, self->exec_graph) == NULL) {
		goto err_return;
	}
	egraph_debug(self->exec_graph, 0);

	pstate_destroy(&self);
	return ret;

err_return:
	printf("%d ERROR: %s\n", self->at, regex_error(self->error));
	pstate_destroy(&self);
	return ret;
}
