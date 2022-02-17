#include <assert.h>
#include <limits.h>

#include "strlx/strlx.h"
#include "regex/errors.h"

#include "dsa.h"
#include "mem.h"
#include "parser.h"

static inline void append_char(strbuf *s, char c)
{
	assert(0 <= c && c <= CHAR_MAX);

	str t = M_str(((char[2]){ c, '\0' }));
	strbuf_append(s, t);
}

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
static int pstate_token_pos(pstate_T const *self, token_T const *tok, int end)
{
	assert(self);
	if (end < 0 || end > self->ntokens)
		end = self->ntokens;

	for (int i = self->at; i < end; i++)
		if (self->tokens[i].type == tok->type &&
		    self->tokens[i].value == tok->value)
			return self->tokens[i].pos;

	return -1;
}

/**
 * @brief Search range [self->at...end), end < 0 => end = self->ntokens
 * 
 * @param self 
 * @param tok 
 * @param end when to stop search, negative for full
 * @return int Index of the matching token in pattern, -1 if not found
 */
static int pstate_token_index(pstate_T const *self, token_T const *tok, int end)
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

	FREE(*eg);
	*eg = NULL;
}

/**
 * @brief Inserts a new node by shallow copying node
 * 
 * @param prev
 * @param node
 * @return egraph_T
 */
static egraph_T *egraph_insert(egraph_T *prev, egraph_T *node)
{
	assert(prev);
	assert(node);

	if (prev->nodecap == prev->nnodes) {
		int newcap = prev->nodecap > 0 ? prev->nodecap * 2 : 1;
		egraph_T *tmp = N_REALLOC(prev->nodes, newcap);
		if (tmp == NULL) {
			free(prev->nodes);
			return NULL;
		}

		prev->nodes = tmp;
		prev->nodecap = newcap;
	}

	egraph_T *new_node = &prev->nodes[prev->nnodes++];
	*new_node = *node;
	new_node->prev = prev;

	return new_node;
}

static void egraph_debug(egraph_T const *eg, int indent)
{
	if (eg->prev != NULL)
		printf("\tP%p__%c -> P%p__%c\n", (void *)eg->prev,
		       eg->prev->value, (void *)eg, eg->value);
	else
		printf("digraph {\n");

	for (int i = 0; i < eg->nnodes; i++) {
		egraph_debug(&eg->nodes[i], indent + 1);
	}
}

static pstate_T *pstate_create(strbuf const *pattern)
{
	assert(pattern);

	pstate_T *ret = ALLOC(ret);
	if (ret == NULL) {
		return NULL;
	}
	token_T *tokens = N_ALLOC(tokens, pattern->size);
	if (tokens == NULL) {
		FREE(ret);
		return NULL;
	}
	pairs_T *parens = pairs_create();
	if (parens == NULL) {
		FREE(ret);
		FREE(tokens);
		return NULL;
	}
	*ret = (pstate_T){
		.tokens = tokens,
		.parens = parens,
		.pattern = pattern,
	};

	return ret;
}

static void pstate_destroy(pstate_T **self)
{
	assert(self);
	pstate_T *tmp = *self;
	assert(tmp);
	assert(tmp->parens);
	assert(tmp->tokens);

	pairs_destroy(&tmp->parens);
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
static int parse_hex_seq(pstate_T *self, token_T *tok)
{
	assert(self);
	assert(tok);

	long long num;
	int const ndigs = 2;
	str hexs = strbuf_substr(self->pattern, self->at, self->at + ndigs);

	if (str_to_ll(hexs, 16, &num) != ndigs)
		return REGEX_INVALID_HEX;
	if (num > CHAR_MAX)
		return REGEX_HEX_TOO_BIG;

	tok->type = RE_TT_ORD;
	tok->value = num;

	tok->chars = strbuf_substr(self->pattern, self->at - tok->chars.size,
				   self->at + ndigs);
	self->at += ndigs;
	return 0;
}

/**
 * @brief Parses number esc-seq, which can be: group num or octal num
 * As decided by the Format:
 * ooo | 0o | 0     => Octal number, (o=octal-digit)
 * 1-99 (inclusive) => Group number
 * 
 * @param self
 * @param tok
 * @return int Error code (success=0)
 */
static int parse_dig_seq(pstate_T *self, token_T *tok)
{
	assert(self);

	long long dec;
	long long oct;
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
		return REGEX_ILLEGAL_CHAR;

	if (pat->data[self->at] == '0' || nocts == 3) {
		assert(0 <= oct && oct <= 0777);
		tok->type = RE_TT_ORD;
		tok->value = oct;
		tok->chars = strbuf_substr(pat, tok_start, self->at + nocts);

		if (oct > CHAR_MAX)
			return REGEX_OCT_TOO_BIG;

		self->at += nocts;
	} else {
		assert(1 <= dec && dec <= 99);
		tok->type = RE_TT_GNUM;
		tok->value = dec;
		tok->chars = strbuf_substr(pat, tok_start, self->at + ndecs);
		self->at += ndecs;
	}

	return 0;
}

static int parse_ctx_tok(token_T *tok, enum re_ctx ctx)
{
	if (ctx == RE_CTX_NONE) {
		switch (tok->type) {
		case RE_TC_CTX_B:
			tok->type = RE_TC_BOUND_WORD;
			tok->value = 0;
			break;

		default:
			assert(!"Illegal CTX token passed");
			break;
		}
	}

	else if (ctx == RE_CTX_BKT) {
		switch (tok->type) {
		case RE_TC_CTX_B:
			tok->type = RE_TC_BS;
			tok->value = RE_TOKENS[RE_TC_BS].value;
			break;

		default:
			assert(!"Illegal CTX token passed");
			break;
		}
	}

	return 0;
}

static int parse_gen_tokens(pstate_T *self)
{
	assert(self);

	strbuf const *pat = self->pattern;

	while (self->at < pat->size) {
		int error = REGEX_NO_MATCH;
		token_T *tok = &self->tokens[self->ntokens++];
		str slice = strbuf_substr(pat, self->at,
					  self->at + RE_MAX_TOKEN_CHARS);

		for (int j = 0; j < RE_NTOKENS; j++) {
			if (!str_starts_with(slice, RE_TOKENS[j].chars))
				continue;

			*tok = RE_TOKENS[j];
			tok->pos = self->at;
			self->at += tok->chars.size;
			error = 0;
			break;
		}

		if (tok->type == RE_TC_HEX)
			error = parse_hex_seq(self, tok);
		// Parse digit escape sequence, as esc digits are not in RE_TOKENS
		else if (tok->type == RE_TC_BSLASH &&
			 str_has_char(STR_DIGITS, pat->data[self->at]))
			error = parse_dig_seq(self, tok);
		// Backslash with no valid escape character
		else if (tok->type == RE_TC_BSLASH)
			error = REGEX_ILLEGAL_CHAR;

		// Other ordinary ASCII characters
		else if (error == REGEX_NO_MATCH && slice.size > 0) {
			*tok = (token_T){
				.value = slice.data[0],
				.type = RE_TT_ORD,
				.chars = str_substr(slice, 0, 1),
				.pos = self->at,
			};
			self->at++;
			error = 0;
		}

		if (error)
			return error;
	}

	self->at = 0;
	assert(self->ntokens <= pat->cap);
	return 0;
}

static int parse_gen_parens_span(pstate_T *self)
{
	assert(self);

	stack_T *stk = stack_create();
	if (stk == NULL)
		return REGEX_NO_MEM;

	int paren = 0; /* 0 if paired */

	for (int i = 0; i < self->ntokens; i++) {
		self->at = i;
		token_T *tok = &self->tokens[i];

		if (tok->type == RE_TC_LPAREN) {
			paren++;

			if (stack_push(stk, i) != 0)
				return REGEX_NO_MEM;
		}

		else if (tok->type == RE_TC_RPAREN) {
			paren--;

			if (paren < 0) {
				stack_destroy(&stk);
				return REGEX_EXTRA_PAREN;
			}
			pairs_insert(self->parens, stack_pop(stk), i);
		}
	}

	if (paren > 0)
		return REGEX_NO_CLOSING_PAREN;

	stack_destroy(&stk);
	self->at = 0;

	return 0;
}

/**
 * @brief Parses range values inside braces{}
 * Format: {N} or {M,N} or {N,} => (N,M = +ve integers, base=10)
 * If not in this format then it({) will be converted to RE_TT_ORD type
 * 
 * @param self 
 * @return int 
 */
static int parse_braces(pstate_T *self, egraph_T *node)
{
	assert(self);
	assert(node);
	assert(self->tokens[self->at].type == RE_TC_LBRACE);

	// Index of metachar '}' in self->tokens
	int end_idx = pstate_token_index(self, &RE_TOKENS[RE_TC_RBRACE], -1);
	// No closing brace or empty
	if (end_idx == -1 || end_idx == self->at + 1) {
		self->tokens[self->at].type = RE_TT_ORD;
		return 0;
	}

	long long min;
	long long max;
	int start = self->tokens[self->at].pos + 1;
	int end = self->tokens[end_idx].pos;
	int sep = pstate_token_pos(
		self, &(token_T){ .value = ',', .type = RE_TT_ORD }, end_idx);

	// Verify format
	for (int i = start; i < end; i++) {
		if (i == sep)
			continue;
		if (!str_has_char(STR_DIGITS, self->pattern->data[i])) {
			self->tokens[self->at].type = RE_TT_ORD;
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

static int parse_char_class(pstate_T *self, egraph_T *node)
{
	assert(self);
	assert(node);

	int inverted = 0;
	token_T *tok = &self->tokens[self->at];

	// Chnage(as needed) and verify token type
	switch (tok->type) {
	case RE_TC_NON_DIGIT:
		inverted = 1;
	case RE_TC_DIGIT:
		tok->type = RE_TC_PCC_DIGIT;
		break;

	case RE_TC_NON_WHITESPACE:
		inverted = 1;
	case RE_TC_WHITESPACE:
		tok->type = RE_TC_PCC_SPACE;
		break;

	case RE_TC_NON_WORD_CHAR:
		inverted = 1;
	case RE_TC_WORD_CHAR:
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
		break;

	default:
		assert(!"Invalid use, non word-class type token");
		break;
	}

	RE_PCC_CHARS[tok->type];

	return 0;
}

/**
 * @brief Parses [.......]
 *
 * Parses following things inside []:
 * Char ranges(R): <char-1>-<char-2> (inclusive), where <char-1> <= <char-2>
 * Char classes(C): \d, \D, \w, \W, \self, \S
 * and Posix char classes(P)
 * 
 * @param self 
 * @return int
 */
static int parse_brackets(pstate_T *self)
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
		      &TOKEN_NEW(RE_TT_ORD, '^'))) {
	}

	return 0;
}

static egraph_T *parse_gen_exec_graph(pstate_T *self, egraph_T *parent,
				      int start, int end)
{
	assert(self);
	assert(parent);

	int err = 0;
	int nmods = 1;
	egraph_T node = { 0 };
	egraph_T *prev = parent;
	self->at = start;

	for (int i = start; i < end; i++) {
		token_T *tok = &self->tokens[i];
		self->at = i;

		if (tok->type == RE_TT_ORD || tok->type == RE_TC_PERIOD) {
			if (tok->type == RE_TC_PERIOD)
				node.anychar = 1;
			else
				node.value = tok->value;

			node.min = 1;
			node.max = 1;

			prev = egraph_insert(prev, &node);
			node = EMPTY_NODE;
			nmods = 0;
			continue;
		}

		switch (tok->type) {
		case RE_TC_ASTERISK:
			nmods++;
			prev->min = 0;
			prev->max = INT_MAX;
			break;

		case RE_TC_PLUS:
			nmods++;
			prev->min = 1;
			prev->max = INT_MAX;
			break;

		case RE_TC_QMARK:
			nmods++;
			prev->min = 0;
			prev->max = 1;
			break;

		case RE_TC_ASTERISK_LAZY:
			nmods++;
			prev->lazy = 1;
			prev->min = 0;
			prev->max = INT_MAX;
			break;

		case RE_TC_PLUS_LAZY:
			nmods++;
			prev->lazy = 1;
			prev->min = 1;
			prev->max = INT_MAX;
			break;

		case RE_TC_QMARK_LAZY:
			nmods++;
			prev->lazy = 1;
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
			if (tok->type == RE_TT_ORD) {
				i = --self->at;
				break;
			} else
				nmods++;

			// If like {...}? then lazy
			if (self->at < self->ntokens &&
			    self->tokens[self->at].type == RE_TC_QMARK) {
				prev->lazy = 1;
				self->at++;
			}
			// As i++ at loop end
			i = --self->at;
			break;

		case RE_TC_RBRACE:
			tok->type = RE_TT_ORD;
			i = --self->at;
			break;

		case RE_TC_LPAREN:;
			int gend = pairs_search(self->parens, i);
			prev = parse_gen_exec_graph(self, prev, i + 1, gend);
			if (prev == NULL)
				return NULL;
			i = self->at;
			break;

		case RE_TC_BAR:
			/* If OR branch then bind prev to parent(root) */
			prev = parent;
			nmods = 1;
			break;

		default:
			break;
		}

		// If more than one consecutive pattern modifiers applied
		// Or if modifier applied to incomplete node(as by default nmods=1)
		if (nmods > 1) {
			self->error = REGEX_ILLEGAL_CHAR;
			return NULL;
		}
	}

	if (parent->nnodes > 0)
		printf("NN: %d\n", parent->nnodes);
	// for (int i = 0; i < parent->nnodes - 1; i++) {
	// 	egraph_T *tmp = &parent->nodes[i];
	// 	while (tmp->nodes != NULL)
	// 		tmp->nodes[0];
	// 	egraph_insert(tmp, prev);
	// }

	return prev;
}

egraph_T *re_parse(strbuf const *pattern)
{
	assert(pattern);

	int error = 0;

	pstate_T *self = pstate_create(pattern);
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

	error = parse_gen_parens_span(self);
	if (error) {
		self->error = error;
		goto err_return;
	}

	if (parse_gen_exec_graph(self, self->exec_graph, 0, self->ntokens) ==
	    NULL) {
		goto err_return;
	}

#ifndef NDEBUG
	egraph_debug(self->exec_graph, 0);
#endif

	pstate_destroy(&self);
	return ret;

err_return:
	printf("%d ERROR: %s\n", self->at, regex_perr(self->error));
	pstate_destroy(&self);
	return ret;
}
