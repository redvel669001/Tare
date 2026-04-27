#ifndef TOKENIZER_H_
#define TOKENIZER_H_

// Lots of things in this header file were copied from nob.h, notably
// the reporting system is largely copied from nob_log
// https://github.com/tsoding/nob.h/blob/main/nob.h

#define TOK_ARG(t) (int) (t)->l, (t)->f

// -------------------------------------------------------------------
// ----------------------- SPECIAL CHARACTERS: -----------------------
// -------------------------------------------------------------------
//
// Special characters are handled by Tare's tokenizer in the following
// way: special characters are enumerated in the `SpecialType` enum,
// which must end with `SPECIAL_TYPES`, begin with the first item set
// to 0, with values incrementing, meaning `SPECIAL_TYPES` acts as a
// counter.
//
// Then a `Specials` character array is made from that enum, with its
// characters indexed in positions matching the enum, and the array
// length being `SPECIAL_TYPES`. While using the enum's counter as the
// array length can prevent SOME errors at copmile time, it doesn't
// necessarily always catch them all, hence the static assert right
// above it.
//
// Additionally, a static assert can be found right above the
// implementation of the `special_index` function, as that's another
// place that should be modified in case the `SpecialType` enum is
// modified.

typedef enum {
  PAR_BGN = 0,
  PAR_END,
  GRP_BGN,
  GRP_END,
  BLK_BGN,
  BLK_END,
  DQUOTE,
  SQUOTE,
  ESC,
  SEP,
  END,
  DOT,
  DEF,
  DIV,
  MOD,
  MULT,
  ADD,
  SUB,
  LESS,
  GREATER,
  EQUAL,
  NOT,
  AND,
  OR,
  SPECIAL_TYPES,
} SpecialType;

static_assert(SPECIAL_TYPES == 24, "Amount of special characters has changed. Please update the `Specials` character array.");
const char Specials[SPECIAL_TYPES] = {
  [PAR_BGN] = '(',
  [PAR_END] = ')',
  [GRP_BGN] = '[',
  [GRP_END] = ']',
  [BLK_BGN] = '{',
  [BLK_END] = '}',
  [DQUOTE]  = '\"',
  [SQUOTE]  = '\'',
  [ESC]     = '\\',
  [SEP]     = ',',
  [END]     = ';',
  [DOT]     = '.',
  [DEF]     = ':',
  [DIV]     = '/',
  [MOD]     = '%',
  [MULT]    = '*',
  [ADD]     = '+',
  [SUB]     = '-',
  [LESS]    = '<',
  [GREATER] = '>',
  [EQUAL]   = '=',
  [NOT]     = '!',
  [AND]     = '&',
  [OR]      = '|',
};

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// ---------------------------- KEYWORDS: ----------------------------
// -------------------------------------------------------------------
//
// Keywords are handled by Tare's tokenizer in the following way:
// keywords are enumerated in the `KeywordType` enum, which must end
// with `KEYWORD_TYPES`, begin with the first item set to 0, with
// values incrementing, meaning `KEYWORD_TYPES` acts as a counter.
//
// Then a `Keywords` string view array is made from that enum, with
// its entries indexed in positions matching the enum, and the array
// length being `KEYWORD_TYPES`. While using the enum's counter as the
// array length can prevent SOME errors at compile time, it doesn't
// necessarily always catch them all, hence the static assert right
// above it.

typedef enum {
  KEY_F = 0,
  KEY_B,
  KEY_N,
  KEY_P,
  KEY_A,
  KEY_S,
  KEY_I,
  KEY_D,
  KEY_R8,
  KEY_R16,
  KEY_R32,
  KEY_R64,
  KEY_IF,
  KEY_WHILE,
  KEY_BREAK,
  KEY_CONT,
  KEY_FUNC,
  KEY_RET,
  KEY_WRITE,
  KEY_READ,
  KEY_SYSCALL,
  KEY_TAPE,
  KEY_HEAD,
  KEY_BASE,
  KEY_INDEX,
  KEY_CONST,

  KEY_PUSH,
  KEY_POP,

  KEY_ADD,
  KEY_SUB,
  KEY_MUL,
  KEY_DIV,
  KEY_MOD,
  KEY_SHL,
  KEY_SHR,
  KEY_NOT,
  KEY_BITWISE_AND,
  KEY_BITWISE_OR,
  KEY_LOGICAL_AND,
  KEY_LOGICAL_OR,
  
  KEY_LESS,
  KEY_LESS_EQUAL,
  KEY_GREATER,
  KEY_GREATER_EQUAL,
  KEY_EQUAL,
  KEY_NOT_EQUAL,
  
  KEY_DEREF,
  
  KEYWORD_TYPES,
} KeywordType;

static_assert(KEYWORD_TYPES == 47, "Amount of keywords has changed. Please update the `Keywords` StringView array.");
StringView Keywords[KEYWORD_TYPES] = {
  [KEY_F]             = SV_MAKE(f),
  [KEY_B]             = SV_MAKE(b),
  [KEY_N]             = SV_MAKE(n),
  [KEY_P]             = SV_MAKE(p),
  [KEY_A]             = SV_MAKE(a),
  [KEY_S]             = SV_MAKE(s),
  [KEY_I]             = SV_MAKE(i),
  [KEY_D]             = SV_MAKE(d),
  [KEY_R8]            = SV_MAKE(r8),
  [KEY_R16]           = SV_MAKE(r16),
  [KEY_R32]           = SV_MAKE(r32),
  [KEY_R64]           = SV_MAKE(r64),
  [KEY_IF]            = SV_MAKE(if),
  [KEY_WHILE]         = SV_MAKE(while),
  [KEY_BREAK]         = SV_MAKE(break),
  [KEY_CONT]          = SV_MAKE(continue),
  [KEY_FUNC]          = SV_MAKE(func),
  [KEY_RET]           = SV_MAKE(ret),
  [KEY_SYSCALL]       = SV_MAKE(syscall),
  [KEY_WRITE]         = SV_MAKE(write),
  [KEY_READ]          = SV_MAKE(read),
  [KEY_TAPE]          = SV_MAKE(tape),
  [KEY_HEAD]          = SV_MAKE(head),
  [KEY_BASE]          = SV_MAKE(base),
  [KEY_INDEX]         = SV_MAKE(index),
  [KEY_CONST]         = SV_MAKE(const),
  
  [KEY_PUSH]          = SV_MAKE(push),
  [KEY_POP]           = SV_MAKE(pop),
  
  [KEY_ADD]           = SV_MAKE(add),
  [KEY_SUB]           = SV_MAKE(sub),
  [KEY_MUL]           = SV_MAKE(mul),
  [KEY_DIV]           = SV_MAKE(div),
  [KEY_MOD]           = SV_MAKE(mod),
  [KEY_SHL]           = SV_MAKE(shl),
  [KEY_SHR]           = SV_MAKE(shr),
  [KEY_NOT]           = SV_MAKE(not),
  [KEY_BITWISE_AND]   = SV_MAKE(bitand),
  [KEY_BITWISE_OR]    = SV_MAKE(bitor),
  [KEY_LOGICAL_AND]   = SV_MAKE(logand),
  [KEY_LOGICAL_OR]    = SV_MAKE(logor),
  
  [KEY_LESS]          = SV_MAKE(less),
  [KEY_LESS_EQUAL]    = SV_MAKE(lequal),
  [KEY_GREATER]       = SV_MAKE(greater),
  [KEY_GREATER_EQUAL] = SV_MAKE(gequal),
  [KEY_EQUAL]         = SV_MAKE(equal),
  [KEY_NOT_EQUAL]     = SV_MAKE(nequal),
  
  [KEY_DEREF]         = SV_MAKE(deref),
};

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// --------------------- BUILTIN VARIABLE TYPES: ---------------------
// -------------------------------------------------------------------
//
// Builtin variable types are handled in the following way: builtin
// variable types are enumerated in the `VariableType` enum, which
// must end with `TYPES_COUNT`, begin with the first item set to 0,
// with values incrementing, meaning `KEYWORD_TYPES` acts as a
// counter.
//
// Then a `VariableTypes` string view array is made from that enum, with
// its entries indexed in positions matching the enum, and the array
// length being `TYPES_COUNT`. While using the enum's counter as the
// array length can prevent SOME errors at compile time, it doesn't
// necessarily always catch them all, hence the static assert right
// above it.

typedef enum {
  TYPE_U8 = 0,
  TYPE_U16,
  TYPE_U32,
  TYPE_U64,
  TYPES_COUNT,
} VariableType;

static_assert(TYPES_COUNT == 4, "Amount of built-in variable types has changed. Please update the `VariableTypes` StringView array.");
StringView VariableTypes[TYPES_COUNT] = {
  SV_MAKE(u8),
  SV_MAKE(u16),
  SV_MAKE(u32),
  SV_MAKE(u64),
};

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// --------------------------- TOKEN TYPES: --------------------------
// -------------------------------------------------------------------
//
// Token types are handled by Tare's tokenizer in the following way:
// token types are enumerated in the `TokenType` enum, which must end
// with `TOKEN_TYPES`, begin with the first item set to 0, with values
// incrementing, meaning `TOKEN_TYPES` acts as a counter.
//
// Then a `TokenTypeNames` string array is made from that enum, with
// its entries indexed in positions matching the enum, and the array
// length being `TYPES_TYPES`. While using the enum's counter as the
// array length can prevent SOME errors at compile time, it doesn't
// necessarily always catch them all, hence the static assert right
// above it.
//
// Additionally, a static assert can be found right above the token
// struct, to ensure any modification intended to be brought about by
// modifying the `TokenType` enum is made after the modification.
//
// Furthermore, a static assert can be found right above the
// implementation of the `fill_tokenizer` function, since that may
// also require modification.

typedef enum {
  TOKEN_TYPE_NAME = 0,
  TOKEN_TYPE_WHOLE_NUM,
  TOKEN_TYPE_FRAC_NUM,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_SPECIAL,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_CHAR,
  
  TOKEN_TYPE_GVID,
  TOKEN_TYPE_LVID,
  TOKEN_TYPE_RVID,
  TOKEN_TYPE_AVID,
  
  TOKEN_TYPE_TID,
  TOKEN_TYPE_FID,
  TOKEN_TYPES,
} TokenType;

static_assert(TOKEN_TYPES == 13, "Amount of token types has changed. Please update the `TokenTypeNames` string array.");
const char *TokenTypeNames[TOKEN_TYPES] = {
  "name",
  "whole number",
  "fractional number",
  "keyword",
  "special",
  "string literal",
  "character literal",
  "variable identifier",
  "type identifier",
  "function identifier",
};

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// --------------------------- LOC STRUCT: ---------------------------
// -------------------------------------------------------------------
//
// The `Loc` struct is used for diagnostics. Since reporting doesn't need to happen very frequently, there's no need to bload the `Token` struct with 16 additional bytes. Rather, the location can be calculated, when necessary, by the 

typedef struct {
  size_t row; // The line at which a token starts.
  size_t col; // How many characters into the line until the token starts.
} Loc;

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// -------------------------- TOKEN STRUCT: --------------------------
// -------------------------------------------------------------------
//
// The `Token` struct is the atomic unit of syntax that tare actually
// understands. This is to say, tokenization in tare is turning text
// into syntax that can more easily be reasoned about.

static_assert(TOKEN_TYPES == 13, "Amount of token types has changed. Please make sure the `Token` struct is working as intended.");
typedef struct {
  TokenType t;     // The token's type.
  
  union {          // This is here because to make padding unnecessary.
    SpecialType s; // The token's index as a special character.
    KeywordType k; // The token's index as a keyword.
  };

  const char *f;   // The first character of the token.
  size_t l;        // The length of the token.

  union {
    size_t u64;    // The token's value as a whole number.
    double f64;    // The token's value as a floating point number.

    size_t jmp;    // The token to which this one is connected - for
                   // example, a `(` token is connected to a `)`
                   // token.
    size_t gvid;   // Token's index as a global variable identifier.
    size_t lvid;   // Token's index as a local variable identifier.
    size_t rvid;   // Token's index as a return value identifier.
    size_t avid;   // Token's index as a function arg identifier.
    size_t tid;    // The token's index as a type identifier. This
                   // would be more useful once a type system is
                   // established, even though for now, it's mostly
                   // unnecessary.
    size_t fid;    // The token's index as a function identifier.
  };
} Token;

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// ------------------------ TOKENIZER STRUCT: ------------------------
// -------------------------------------------------------------------
//
// The `Tokenizer` struct acts as a fancy dynamic array, in a
// sense. It is mostly used for looping over the tokens while keeping
// track of the current token and its index, other than during
// tokenization, when the lexer is more like a fancy dynamic
// array.

typedef struct {
  Token *items;     // The tokenizer's tokens. This is a dynamic array.
  size_t count;     // The amount of current tokens.
  size_t capacity;  // How much space has been allocated for tokens.
  Token *t;         // The current token the tokenizer points at. This
                    // should be equivalent to (items + index).
  size_t index;     // The tokenizer's current index. This should be
                    // equivalent to (size_t) (t - items).
  const char *path; // The path to the file that has been read for tokenization.
  Lexer l;          // The lexer used for tokenization.
} Tokenizer;

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// -------------------------- REPORT LEVEL: --------------------------
// -------------------------------------------------------------------
//
// The `ReportLevel` enum is really only important for printing
// diagnostics, e.g. when encountering an error.

typedef enum {
  REPORT_NOTE,
  REPORT_WARNING,
  REPORT_ERROR,
} ReportLevel;

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// ---------------------- TOKENIZATION FUNCTIONS: --------------------
// -------------------------------------------------------------------
//
// These three functions concern tokenization directly, and should be
// unimportant beyond that stage.

// For a character c, if it is equal to some special character, return
// that special character's index in the `Specials` character
// array. If it isn't present, simply return SPECIAL_TYPES to indicate
// failure.
TARE_DEF SpecialType special_index(char c);

// Read file `path` and lexically analyze it to fill tokenizer
// `t`. Return true to indicate success, false to indicate
// failure. Failure could happen either because `path` doesn't exist,
// or because the text contains an invalid token, or otherwise, a
// failure could happen because of a failure on the part of the lexer.
TARE_DEF bool tokenize_file(const char *path, Tokenizer *t);

// Assuming `t` has its lexer prepared, use it to read into the text
// and fill `t` with tokens. Return true to indicate success, false to
// indicate failure. Failure could happen because of invalid tokens in
// the text parsed, or because of a lexer error.
TARE_DEF bool fill_tokenizer(Tokenizer *t);

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// -------------------------- DIAGNOSTICS: ---------------------------
// -------------------------------------------------------------------
//
// These functions and macros should be used for reporting errors,
// warnings, or notes, or otherwise for debugging purposes.

// Get the row and col of token `tok` from tokenizer `t`, in the form
// of struct `Loc`.
TARE_DEF Loc get_token_loc(const Tokenizer *t, const Token *tok);

// Print the location of token `tok` from tokenizer `t`, to file
// `stream`, in the format `FILE:ROW:COL: ` (including the whitespace
// at the end).
TARE_DEF void print_loc(FILE *stream, const Tokenizer *t, const Token *tok);

// For valid `r`, pick an appropriate output file (`stream`) and
// an appropriate preamble string (`diag`):
//
// REPORT_NOTE: stream = stdout, diag = "note: ",
// REPORT_WARNING: stream = stderr, diag = "warning: ",
// REPORT_ERROR: stream = stderr, diag = "error: ",
//
// Print the location of `tok` from tokenizer `t` to `stream` (via
// `print_loc`), then print `diag` to `stream`, then print the
// formatted message `fmt` and the formatting arguments following it.
//
// NOTE: this function can be inconvenient to use due to its excessive
// parameter count, but there are macros to make usage of it easier,
// and those should probably be used instead.
TARE_DEF void report(ReportLevel r, const Tokenizer *t, const Token *tok, const char *fmt, ...) __attribute__ ((format (printf, 4, 5)));

// Print a note diagnosis for token `tok` from tokenizer `t` with
// formatted message `fmt` and the formatting arguments following it.
#define diag_notef(t, tok, fmt, ...) \
  report(REPORT_NOTE, (t), (tok), (fmt), __VA_ARGS__)

// Print a warning diagnosis for token `tok` from tokenizer `t` with
// formatted message `fmt` and the formatting arguments following it.
#define diag_warnf(t, tok, fmt, ...) \
  report(REPORT_WARNING, (t), (tok), (fmt), __VA_ARGS__)

// Print a error diagnosis for token `tok` from tokenizer `t` with
// formatted message `fmt` and the formatting arguments following it.
#define diag_errf(t, tok, fmt, ...) \
  report(REPORT_ERROR, (t), (tok), (fmt), __VA_ARGS__)

// Print an unformatted note diagnosis `msg` for token `tok` from
// tokenizer `t`.
#define diag_note(t, tok, msg, ...) report(REPORT_NOTE, (t), (tok), (msg))

// Print an unformatted warning diagnosis `msg` for token `tok` from
// tokenizer `t`.
#define diag_warn(t, tok, msg, ...) report(REPORT_WARNING, (t), (tok), (msg))

// Print an unformatted error diagnosis `msg` for token `tok` from
// tokenizer `t`.
#define diag_err(t, tok, msg, ...) report(REPORT_ERROR, (t), (tok), (msg))

// Print token `tok` from tokenizer `t` to stdout (via `print_loc`),
// with some helpful information such as the token's type and the
// value of the field relevant to that token type.
TARE_DEF void debug_print_token(const Tokenizer *t, const Token *tok);

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// ---------------------- COMPARISON FUNCTIONS: ----------------------
// -------------------------------------------------------------------
//
// These functions are useful for pre-parsing tokenizer preparations,
// comparing tokens to other tokens, or to a string view.

// Compare token `tok` with string view `sv`. Return true if the
// contents are identical (same length AND same characters for that
// length), return false otherwise.
TARE_DEF bool tok_sv_cmp(Token *tok, StringView sv);

// Compare token `t1` with token `t2`. Return true if the contents are
// identical (same length AND same characters for that length), return
// false otherwise.
TARE_DEF bool tok_eq(Token *t1, Token *t2);

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// ---------------------- ITERATING FUNCTIONS: -----------------------
// -------------------------------------------------------------------
//
// These functions should be used to jump around the tokenizer. Useful
// for pre-parsing preparation AND for parsing.

// Change tokenizer `t`'s index to `index` and if it is within bounds, set
// the current token to the one matching the index.
TARE_DEF bool to_token(Tokenizer *t, size_t index);

// Change tokenizer `t`'s index to 0 and set the current token to the
// first one,
//
// NOTE: This should be equivalent to `to_token(t, 0)`.
TARE_DEF bool first_token(Tokenizer *t);

// Change tokenizer `t`'s index to one more than the current index and
// if it is within bounds, set the current token to the one after the
// one currently pointed at.
//
// NOTE: This should be equivalent to `to_token(t, t->index + 1)`.
TARE_DEF bool next_token(Tokenizer *t);

// Change tokenizer `t`'s index to one less than the current index and
// if it is within bounds, set the current token to the one before the
// one currently pointed at.
//
// NOTE: This should be equivalent to `to_token(t, t->index - 1)`.
TARE_DEF bool prev_token(Tokenizer *t);

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// ----------------------- PEEKING FUNCTIONS: ------------------------
// -------------------------------------------------------------------
//
// These functions are useful for looking ahead and looking backwards,
// without actually committing to jumping around, to better assess the
// situation while jumping around in expectation functions,
// pre-parsing tokenizer preparations, or while parsing.

// Check if an index one less than tokenizer `t`'s index is within
// bounds. If it is, return the token at that prior index by
// value. Otherwise, return a 0-initialized token to indicate error.
TARE_DEF Token peek_prev_token(const Tokenizer *t);

// Check if an index one more than tokenizer `t`'s index is within
// bounds. If it is, return the token at that next index by
// value. Otherwise, return a 0-initialized token to indicate error.
TARE_DEF Token peek_next_token(const Tokenizer *t);

// Check if an index `forward` more than tokenizer `t`'s index is
// within bounds. If it is, return the token at that forward index by
// value. Otherwise, return a 0-initialized token to indicate error.
TARE_DEF Token peek_forward_token(const Tokenizer *t, size_t forward);

// -------------------------------------------------------------------

// -------------------------------------------------------------------
// --------------------------- EXPECTATIONS: -------------------------
// -------------------------------------------------------------------
//
// These functions and macros are useful for defining syntax, meaning
// they are useful during pre-parsing tokenizer preparations and
// parsing.

// Expect tokenizer `t`'s next token to be of type `type`. Return true
// to indicate success, false to indicate failure. Failure can happen
// either because there is no next token, or because the next token's
// type isn't equal to `type`.
TARE_DEF bool expect_token_type(Tokenizer *t, TokenType type);

// Expect tokenizer `t`'s next token to be of type `a` or `b`. Return
// true to indicate success, false to indicate failure. Failure can
// happen either because there is no next token, or because the next
// token's type isn't equal to `a` AND isn't equal to `b`.
TARE_DEF bool expect_token_type_two(Tokenizer *t, TokenType a, TokenType b);

// Expect tokenizer `t`'s next token to be of type `a`, `b`, or
// `c`. Return true to indicate success, false to indicate
// failure. Failure can happen either because there is no next token,
// or because the next token's type isn't equal to `a`, AND isn't
// equal to `b`, AND isn't equal to `c`.
TARE_DEF bool expect_token_type_three(Tokenizer *t, TokenType a, TokenType b, TokenType c);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_NAME`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_NAME`.
TARE_DEF bool expect_name(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_WHOLE_NUM`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_WHOLE_NUM`.
TARE_DEF bool expect_whole_num(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_FRAC_NUM`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_FRAC_NUM`.
TARE_DEF bool expect_frac_num(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_KEYWORD`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_KEYWORD`.
TARE_DEF bool expect_keyword(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_SPECIAL` and expect that next token's special type to
// be equal to `s``. Return true to indicate success, false to
// indicate failure. Failure can happen because there is no next
// token, or because the next token's type isn't equal to
// `TOKEN_TYPE_SPECIAL`, or because that next token's special type
// isn't equal to `s`.
TARE_DEF bool expect_special(Tokenizer *t, SpecialType s);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_SPECIAL` and expect that next token's special type to
// be equal to one of the special type passed by argument to the
// function (excluding `SPECIAL_TYPES`, as it is used to indicate the
// end of the variadic arguments). Return true to indicate success,
// false to indicate failure. Failure can happen because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_SPECIAL`, or because that next token's special type
// isn't equal to any of the special types passed by argument to the
// function (excluding `SPECIAL_TYPES`, as it is used to indicate the
// end of the variadic arguments).

// NOTE: This function expects the variadic arguments passed to end
// with the invalid special type index `SPECIAL_TYPES`. For a more
// convenient API, use the `expect_special_many` macro instead, as it
// adds the terminating value for you.
TARE_DEF bool expect_special_many_function(Tokenizer *t, ...);

// Expect tokenizer `t`'s next tokens to be of type
// `TOKEN_TYPE_SPECIAL` and expect these next tokens' special types to
// be equal to the special types passed by argument to the function,
// in sequence (excluding `SPECIAL_TYPES`, as it is used to indicate
// the end of the variadic arguments). Return true to indicate
// success, false to indicate failure. Failure can happen because
// there aren't as many next tokens as there are special types passed
// by argument to the function (excluding `SPECIAL_TYPES`, as it is
// used to indicate the end of the variadic arguments), or otherwise
// because one of the next tokens fails the expectation - either
// because its type isn't `TOKEN_TYPE_SPECIAL`, or because it is but
// its special type isn't equal to matching one in the special types
// passed by argument to the function (excluding `SPECIAL_TYPES`, as
// it is used to indicate the end of the variadic arguments).

// NOTE: This function expects the variadic arguments passed to end with the
// invalid special type index `SPECIAL_TYPES`. For a more convenient
// API, use the `expect_special_many` macro instead, as it adds the
// terminating value for you.
TARE_DEF bool expect_special_sequence_function(Tokenizer *t, ...);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_CHAR`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_CHAR`.
TARE_DEF bool expect_char(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_VID`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_VID`.
/* TARE_DEF bool expect_vid(Tokenizer *t); */

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_TID`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_TID`.
TARE_DEF bool expect_tid(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type
// `TOKEN_TYPE_FID`. Return true to indicate success, false to
// indicate failure. Failure can happen either because there is no
// next token, or because the next token's type isn't equal to
// `TOKEN_TYPE_FID`.
TARE_DEF bool expect_fid(Tokenizer *t);

// Expect tokenizer `t`'s next token to be of type `TOKEN_TYPE_TID` OR
// `TOKEN_TYPE_KEYWORD` with keyword type of `KEY_CONST`. Return true
// to indicate success, false to indicate failure. Failure can happen
// either because there is no next token, or because the next token
// isn't of type `TOKEN_TYPE_TID` AND either isn't of
// `TOKEN_TYPE_KEYWORD` or is but its keyword type isn't `KEY_CONST`.
TARE_DEF bool expect_tid_or_const(Tokenizer *t);

#define expect_special_many(t, ...)                             \
  expect_special_many_function((t), __VA_ARGS__, SPECIAL_TYPES)

#define expect_special_sequence(t, ...)                                 \
  expect_special_sequence_function((t), __VA_ARGS__, SPECIAL_TYPES)

// -------------------------------------------------------------------

#endif // TOKENIZER_H_

#ifdef TOKENIZER_IMPLEMENTATION

static_assert(SPECIAL_TYPES == 24, "Amount of special characters has changed. Please update the `special_index` function to properly account for that.");
TARE_DEF SpecialType special_index(char c) {
  switch (c) {
  case '(': return PAR_BGN;
  case ')': return PAR_END;
  case '[': return GRP_BGN;
  case ']': return GRP_END;
  case '{': return BLK_BGN;
  case '}': return BLK_END;
  case '\"': return DQUOTE;
  case '\'': return SQUOTE;
  case '\\': return ESC;
  case ',': return SEP;
  case ';': return END;
  case '.': return DOT;
  case ':': return DEF;
  case '/': return DIV;
  case '%': return MOD;
  case '*': return MULT;
  case '+': return ADD;
  case '-': return SUB;
  case '<': return LESS;
  case '>': return GREATER;
  case '=': return EQUAL;
  case '!': return NOT;
  case '&': return AND;
  case '|': return OR;
  default: return SPECIAL_TYPES;
  }
}

TARE_DEF bool tokenize_file(const char *path, Tokenizer *t) {
  if (t == NULL || path == NULL) return false; // Sanity check.
  t->path = path;
  /* if (!read_file(path, &t->l.buf)) return false; */
  if (!read_file_to_lexer(path, &t->l)) return false;
  return fill_tokenizer(t); // Tokenize the text.
}

static_assert(TOKEN_TYPES == 13, "Amount of token types has changed. Please update the `fill_tokenizer` function, or otherwise make sure it's working as intended.");
TARE_DEF bool fill_tokenizer(Tokenizer *t) {
  if (t == NULL || t->l.items == NULL) return false; // Sanity check.
  first_char(&t->l); // Just in case.
  do { // Do while cause it fits this use case.
    if (isspace(*t->l.c)) continue; // Skip whitespace.
    
    // Token still mostly zero-initialized, but with some fields
    // filled in to avoid needless repetition.
    Token tok = {.f = t->l.c, .l = 1, .s = special_index(*t->l.c)};

    // Try to look ahead fo check for an indication of a fractional
    // number. May expand to look for other indications of a number.
    bool is_frac = is_digit(peek_next_char(&t->l)) && tok.s == DOT;
    if (tok.s == DQUOTE) { // Handle string literal.
      tok.t = TOKEN_TYPE_STRING;
      // First character pointed at the dqoute, but should point right
      // after it. Similarly, the length, the first character, the
      // row, and the col, should also reflect that fact.
      tok.f++, tok.l--;
      if (!next_char(&t->l)) return false;
      while (*t->l.c != Specials[DQUOTE]) { // Handle the rest of the string.
        tok.l++;
        if (!next_char(&t->l)) break;
        // Skip escaped charcters to not end the string too early.
        if (*t->l.c == Specials[ESC]) {
          if (!next_char(&t->l)) break;
          tok.l++;
          if (!next_char(&t->l)) break;
          tok.l++;
        }
      }
      da_append(t, tok);
      continue;
    } else if (tok.s == SQUOTE) { // Handle character literal.
      tok.t = TOKEN_TYPE_CHAR;
      // First character pointed at the squote, but should point right
      // after it. Similarly, the first character, the row, and the
      // col, should also reflect that.
      tok.f++;
      if (!next_char(&t->l)) return false;
      if (*t->l.c == Specials[ESC]) { // Check for escaped characters.
        if (!next_char(&t->l)) return false;
        tok.l++; // Increase length to capture the whole thing.
      }
      if (!next_char(&t->l)) return false;
      if (*t->l.c != Specials[SQUOTE]) return false;
      da_append(t, tok);
      continue;
    } else if (tok.s < SPECIAL_TYPES && !is_frac) {
      // Handle special characters.
      if (tok.s == DIV) { // Check for comments
        SpecialType next = special_index(peek_next_char(&t->l));
        if (next == DIV) { // Single-line comment.
          while (next_char(&t->l)) if (*t->l.c == '\n') break;
          continue;
        } else if (next == MULT) { // Multi-line comment.
          while (next_char(&t->l)) {
            SpecialType prev = special_index(peek_prev_char(&t->l));
            if (prev == MULT && special_index(*t->l.c) == DIV) break;
          }
          continue;
        }
      }
      tok.t = TOKEN_TYPE_SPECIAL;
      da_append(t, tok);
      continue;
    }

    // Numeric or name token.
    bool is_whole = is_digit(*tok.f); // Prepare for numeric token.
    double frac = 0;
    double frac_mult = 1;
    if (is_whole) tok.u64 = tok.u64 * 10 + (*tok.f - '0');
    while (special_index(peek_next_char(&t->l)) >= SPECIAL_TYPES) {
      // Tokenize the rest of the token.
      if (!next_char(&t->l)) break;
      if (isspace(*t->l.c)) break;
      tok.l++;
      is_whole &= is_digit(*t->l.c);
      if (is_whole) tok.u64 = tok.u64 * 10 + (*t->l.c - '0');
      if (is_whole && special_index(peek_next_char(&t->l)) == DOT) {
        if (is_frac) return false;
        is_frac = true;
        is_whole = false;
        if (!next_char(&t->l)) break; // Skip the dot.
        tok.l++;
        if (!next_char(&t->l)) break; // Digit after the dot.
        tok.l++;
      }
      if (is_frac) {
        frac = frac * 10 + *t->l.c - '0';
        frac_mult *= 10;
        is_whole = false;
      }
    }
    if (!is_whole && !is_frac) { // Handle name tokens.
      tok.t = TOKEN_TYPE_NAME;
      tok.u64 = 0;
      for (size_t i = 0; i < KEYWORD_TYPES; i++) { // Handle keyword tokens.
        if (tok_sv_cmp(&tok, Keywords[i])) {
          tok.t = TOKEN_TYPE_KEYWORD;
          tok.k = i;
          break;
        }
      }
    } else { // Handle numeric tokens.
      if (is_whole) tok.t = TOKEN_TYPE_WHOLE_NUM;
      else { // Handle fractional tokens.
        tok.t = TOKEN_TYPE_FRAC_NUM;
        frac /= frac_mult;
        frac += tok.u64;
        tok.f64 = frac;
      }
    }
    da_append(t, tok);
  } while (next_char(&t->l));

  return true;
}

TARE_DEF Loc get_token_loc(const Tokenizer *t, const Token *tok) {
  Loc loc = {.row = 1, .col = 1};
  for (const char *point = t->l.items; point < tok->f; point++) {
    if (*point == '\n') {
      loc.row++;
      loc.col = 1;
      continue;
    }
    loc.col++;
  }
  return loc;
}

TARE_DEF void print_loc(FILE *stream, const Tokenizer *t, const Token *tok) {
  Loc loc = get_token_loc(t, tok);
  fprintf(stream, "%s:%zu:%zu: ", t->path, loc.row, loc.col);
}

TARE_DEF void report(ReportLevel r, const Tokenizer *t, const Token *tok, const char *fmt, ...) {
  FILE *stream;
  const char *diag = "";
  switch (r) {
  case REPORT_NOTE:
    stream = stdout;
    diag = "note";
    break;
  case REPORT_WARNING:
    stream = stderr;
    diag = "warning";
    break;
  case REPORT_ERROR:
    stream = stderr;
    diag = "error";
    break;
  default:
    stream = stderr;
    fprintf(stream, "WRONG USAGE!\n");
    exit(1);
    break;
  }

  print_loc(stream, t, tok);
  fprintf(stream, "%s: ", diag);
  va_list args;
  va_start(args, fmt);
  vfprintf(stream, fmt, args);
  va_end(args);
}

TARE_DEF void debug_print_token(const Tokenizer *t, const Token *tok) {
  print_loc(stdout, t, tok);
  switch (tok->t) {
  case TOKEN_TYPE_NAME: printf("(name) %.*s\n", TOK_ARG(tok)); break;
  case TOKEN_TYPE_WHOLE_NUM: printf("(whole) %zu\n", tok->u64); break;
  case TOKEN_TYPE_FRAC_NUM: printf("(frac) %lf\n", tok->f64); break;
  case TOKEN_TYPE_KEYWORD: printf("(key) %.*s\n", TOK_ARG(tok)); break;
  case TOKEN_TYPE_SPECIAL: printf("(special) %.*s\n", TOK_ARG(tok)); break;
  case TOKEN_TYPE_STRING: printf("(string) %.*s\n", TOK_ARG(tok)); break;
  case TOKEN_TYPE_CHAR: printf("(char) %.*s\n", TOK_ARG(tok)); break;
    
  case TOKEN_TYPE_GVID:
    printf("(gvid) %.*s (%zu)\n", TOK_ARG(tok), tok->gvid); break;
  case TOKEN_TYPE_LVID:
    printf("(lvid) %.*s (%zu)\n", TOK_ARG(tok), tok->lvid); break;
  case TOKEN_TYPE_RVID:
    printf("(rvid) %.*s (%zu)\n", TOK_ARG(tok), tok->rvid); break;
  case TOKEN_TYPE_AVID:
    printf("(avid) %.*s (%zu)\n", TOK_ARG(tok), tok->avid); break;
    
  case TOKEN_TYPE_TID:
    printf("(tid) %.*s (%zu)\n", TOK_ARG(tok), tok->tid); break;
  case TOKEN_TYPE_FID:
    printf("(fid) %.*s (%zu)\n", TOK_ARG(tok), tok->fid); break;
  case TOKEN_TYPES: default: assert(false && "unreachable");
  }
}

TARE_DEF bool tok_sv_cmp(Token *tok, StringView sv) {
  if (tok->l != sv.l) return false;
  return strncmp(tok->f, sv.s, sv.l) == 0;
}

TARE_DEF bool tok_eq(Token *t1, Token *t2) {
  if (t1->l != t2->l) return false;
  return strncmp(t1->f, t2->f, t1->l) == 0;
}

TARE_DEF bool to_token(Tokenizer *t, size_t index) {
  t->t = t->items + index;
  t->index = index;
  return check_bounds(index, t->count);
}

TARE_DEF bool first_token(Tokenizer *t) {
  return to_token(t, 0);
}

TARE_DEF bool next_token(Tokenizer *t) {
  return to_token(t, t->index + 1);
}

TARE_DEF bool prev_token(Tokenizer *t) {
  return to_token(t, t->index - 1);
}

TARE_DEF Token peek_prev_token(const Tokenizer *t) {
  if (!check_bounds(t->index - 1, t->count)) return (Token) {0};
  return t->items[t->index - 1];
}

TARE_DEF Token peek_next_token(const Tokenizer *t) {
  if (!check_bounds(t->index + 1, t->count)) return (Token) {0};
  return t->items[t->index + 1];
}

TARE_DEF Token peek_forward_token(const Tokenizer *t, size_t forward) {
  if (!check_bounds(t->index + forward, t->count)) return (Token) {0};
  return t->items[t->index + forward];
}

TARE_DEF bool expect_token_type(Tokenizer *t, TokenType type) {
  if (next_token(t)) {
    if (t->t->t == type) return true;
    diag_errf(t, t->t, "expected %s, but got %s instead\n",
            TokenTypeNames[type], TokenTypeNames[t->t->t]);
    return false;
  }
  if (!prev_token(t)) {
    fprintf(stderr, "TOKENIZER error!\n");
  }
  diag_errf(t, t->t, "expected %s after `%.*s`, but it's the last token.\n",
          TokenTypeNames[type], TOK_ARG(t->t));
  return false;
}

TARE_DEF bool expect_token_type_two(Tokenizer *t, TokenType a, TokenType b) {
  if (next_token(t)) {
    if (t->t->t == a || t->t->t == b) return true;
    diag_errf(t, t->t, "expected %s or %s, but got %s instead\n",
            TokenTypeNames[a], TokenTypeNames[b], TokenTypeNames[t->t->t]);
    return false;
  }
  if (!prev_token(t)) {
    fprintf(stderr, "TOKENIZER error!\n");
  }
  diag_errf(t, t->t,
            "expected %s or %s after `%.*s`, but it's the last token.\n",
            TokenTypeNames[a], TokenTypeNames[b], TOK_ARG(t->t));
  return false;
}

TARE_DEF bool expect_token_type_three(Tokenizer *t, TokenType a, TokenType b, TokenType c) {
  if (next_token(t)) {
    if (t->t->t == a || t->t->t == b || t->t->t == c) return true;
    diag_errf(t, t->t, "expected %s, %s, or %s, but got %s instead\n",
            TokenTypeNames[a], TokenTypeNames[b], TokenTypeNames[c],
            TokenTypeNames[t->t->t]);
    return false;
  }
  if (!prev_token(t)) {
    fprintf(stderr, "TOKENIZER error!\n");
  }
  diag_errf(t, t->t, "error: expected %s, %s, or %s after `%.*s`, but it's the last token.\n",
            TokenTypeNames[a], TokenTypeNames[b], TokenTypeNames[c],
            TOK_ARG(t->t));
  return false;
}

TARE_DEF bool expect_name(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_NAME);
}

TARE_DEF bool expect_whole_num(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_WHOLE_NUM);
}

TARE_DEF bool expect_frac_num(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_FRAC_NUM);
}

TARE_DEF bool expect_keyword(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_KEYWORD);
}

TARE_DEF bool expect_special(Tokenizer *t, SpecialType s) {
  if (!expect_token_type(t, TOKEN_TYPE_SPECIAL)) t->t->s = SPECIAL_TYPES;
  if (t->t->s == s) return true;
  diag_errf(t, t->t, "expected `%c` but got `%.*s` instead.\n",
            Specials[s], TOK_ARG(t->t));
  return false;
}

TARE_DEF bool expect_special_many_function(Tokenizer *t, ...) {
  if (!expect_token_type(t, TOKEN_TYPE_SPECIAL)) t->t->s = SPECIAL_TYPES;
  size_t count = 0;
  va_list args;
  va_start(args, t);
  
  SpecialType s = va_arg(args, SpecialType);

  while (s < SPECIAL_TYPES) {
    if (t->t->s == s) return true;
    count++;
    s = va_arg(args, SpecialType);
  }

  va_end(args);
  va_start(args, t);

  s = va_arg(args, SpecialType);

  diag_err(t, t->t, "expected ");
  for (size_t i = 0; i < count; i++) {
    const char ch = Specials[s];
    fprintf(stderr, "`%c`", ch);
    if (i + 2 == count) {
      s = va_arg(args, SpecialType);
      const char cha = Specials[s];
      fprintf(stderr, " or `%c` ", cha);
      break;
    } else fprintf(stderr, ", ");
    s = va_arg(args, SpecialType);
  }
  fprintf(stderr, "but got %s `%.*s` instead.\n",
          TokenTypeNames[t->t->t], TOK_ARG(t->t));
  
  return false;
}

TARE_DEF bool expect_special_sequence_function(Tokenizer *t, ...) {
  size_t count = 0;
  va_list args;
  va_start(args, t);
  
  while (va_arg(args, SpecialType) < SPECIAL_TYPES) count++;

  va_end(args);
  va_start(args, t);

  SpecialType s = va_arg(args, SpecialType);

  Token *begin = t->t;

  bool success = true;

  for (size_t i = 0; success && (i < count); i++) {
    if (!expect_token_type(t, TOKEN_TYPE_SPECIAL)) success = false;
    if (t->t->s != s) success = false;
    s = va_arg(args, SpecialType);
  }

  if (success) return true;

  va_end(args);
  va_start(args, t);
  
  s = va_arg(args, SpecialType);
  
  diag_err(t, begin, "expected ");
  for (size_t i = 0; i < count; i++) {
    const char ch = Specials[s];
    fprintf(stderr, "`%c`", ch);
    if (i + 2 == count) {
      s = va_arg(args, SpecialType);
      const char cha = Specials[s];
      fprintf(stderr, " and then `%c` ", cha);
      break;
    } else fprintf(stderr, ", ");
    s = va_arg(args, SpecialType);
  }
  fprintf(stderr, "but got %s `%.*s` instead.\n",
          TokenTypeNames[t->t->t], TOK_ARG(t->t));

  va_end(args);

  return false;
}

TARE_DEF bool expect_char(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_CHAR);
}

/* TARE_DEF bool expect_vid(Tokenizer *t) { */
/*   return expect_token_type(t, TOKEN_TYPE_VID); */
/* } */

TARE_DEF bool expect_tid(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_TID);
}

TARE_DEF bool expect_fid(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_FID);
}

TARE_DEF bool expect_tid_or_const(Tokenizer *t) {
  if (!expect_token_type_two(t, TOKEN_TYPE_TID, TOKEN_TYPE_KEYWORD)) {
    diag_errf(t, t->t, "expected type identifier or keyword `%.*s` but got %.*s instead\n", SV_ARG(Keywords[KEY_CONST]), TOK_ARG(t->t));
    return false;
  }

  if (t->t->t == TOKEN_TYPE_TID) return true;
  if (t->t->k == KEY_CONST) return true;
  
  diag_errf(t, t->t, "expected type identifier or keyword `%.*s` but got %.*s instead\n", SV_ARG(Keywords[KEY_CONST]), TOK_ARG(t->t));
  return false;
}

#endif // TOKENIZER_IMPLEMENTATION

