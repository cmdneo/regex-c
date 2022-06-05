#ifndef REGEX_PARSER_H_INTERNAL
#define REGEX_PARSER_H_INTERNAL

#include "strlx/strlx.h"
#include "dsa.h"

/* -- Data structures -- */
typedef struct token_T token_T;
typedef struct egraph_T egraph_T;

/**
 * @brief Execution graph
 */
struct egraph_T {
	unsigned dead : 1;
	unsigned lazy : 1;
	unsigned capture : 1;
	unsigned atomic : 1;
	unsigned anyone : 1;
	unsigned anychar : 1;
	unsigned is_group : 1;
	unsigned is_cclass : 1;
	unsigned is_class_inv : 1;
	int error;
	int min;
	int max;
	int value;
	int nmatches;
	int nnodes;
	int nodecap;
	strbuf *cclass_chars; /** if is_cclass, otherwise NULL */
	egraph_T *nodes; /** last node is the next node */
	egraph_T *prev;
};

typedef struct pstate_T {
	int error;
	int ntokens;
	int ctx;
	int at; /** tracker */
	egraph_T *exec_graph;
	pairs_T *parens;
	token_T *tokens;
	strbuf const *pattern;
} pstate_T;

typedef struct regex {
	strbuf const *pattern;
	egraph_T *exec_graph;
} regex;

/*  -- Functions -- */

#define EGDATA_SIZE(n) (sizeof(egdata_T) + sizeof(egraph_T[(n)]))
#define TOKEN_NEW(typ, val) ((token_T){ .type = typ, .value = val })
egraph_T *re_parse(strbuf const *pattern);

/* -- Config & Data -- */

#define EMPTY_NODE ((egraph_T){ 0 })
static const str RE_CTX_CHARS = M_str("b");

enum re_extension {
	RE_EXT_ATOMIC,
	RE_EXT_GNAME,
	/* Number of extensions */
	RE_EXT_COUNT
};

/* Extension prefixes must start with a question-mark(?) */
static const str re_ext_prefixes[RE_EXT_COUNT] = {
	[RE_EXT_ATOMIC] = M_str("?>"),
	[RE_EXT_GNAME] = M_str("?P"),
};

#endif
