#ifndef TOKENIZER_H_
#define TOKENIZER_H_

// Lots of things in this header file were copied from nob.h, notably
// the reporting system is largely copied from nob_log
// https://github.com/tsoding/nob.h/blob/main/nob.h

#define TOK_ARG(t) (int) (t)->l, (t)->f

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

static_assert(SPECIAL_TYPES == 23, "Amount of special characters has changed. Please update the `Specials` character array (string).");
const char Specials[SPECIAL_TYPES] = "()[]{}\"\'\\,;.:/*+-<>=!&|";

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

static_assert(KEYWORD_TYPES == 46, "Amount of keywords has changed. Please update the `Keywords` StringView array.");
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

typedef enum {
  TYPE_U8 = 0,
  TYPE_U16,
  TYPE_U32,
  TYPE_U64,
  TYPES_COUNT,
} VariableType;

StringView VariableTypes[TYPES_COUNT] = {
  SV_MAKE(u8),
  SV_MAKE(u16),
  SV_MAKE(u32),
  SV_MAKE(u64),
};

typedef enum {
  TOKEN_TYPE_NAME = 0,
  TOKEN_TYPE_WHOLE_NUM,
  TOKEN_TYPE_FRAC_NUM,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_SPECIAL,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_CHAR,
  TOKEN_TYPE_VID,
  TOKEN_TYPE_TID,
  TOKEN_TYPE_FID,
  TOKEN_TYPES,
} TokenType;

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

typedef struct {
  TokenType t;
  const char *f;
  size_t l;

  union {
    size_t u64;
    double f64;
  };

  SpecialType s;
  KeywordType k;
  
  char c;
  size_t row, col;
  size_t jmp;
  size_t jmp2;
  size_t vid;
  size_t tid;
  size_t fid;
} Token;

typedef struct {
  Token *items;
  size_t count, capacity;
  Token *t;
  size_t index;
  const char *path;
  Lexer l;
} Tokenizer;

typedef enum {
  REPORT_NOTE,
  REPORT_WARNING,
  REPORT_ERROR,
} ReportLevel;

TARE_DEF SpecialType special_index(char c);
TARE_DEF bool tokenize_file(const char *path, Tokenizer *t);
TARE_DEF bool fill_tokenizer(Tokenizer *t);

TARE_DEF void print_loc(FILE *stream, const Tokenizer *t, const Token *tok);
TARE_DEF void report(ReportLevel r, const Tokenizer *t, const Token *tok, const char *fmt, ...) __attribute__ ((format (printf, 4, 5)));

#define diag_notef(t, tok, fmt, ...) \
  report(REPORT_NOTE, (t), (tok), (fmt), __VA_ARGS__)
#define diag_warnf(t, tok, fmt, ...) \
  report(REPORT_WARNING, (t), (tok), (fmt), __VA_ARGS__)
#define diag_errf(t, tok, fmt, ...) \
  report(REPORT_ERROR, (t), (tok), (fmt), __VA_ARGS__)

#define diag_note(t, tok, fmt, ...) report(REPORT_NOTE, (t), (tok), (fmt))
#define diag_warn(t, tok, fmt, ...) report(REPORT_WARNING, (t), (tok), (fmt))
#define diag_err(t, tok, fmt, ...) report(REPORT_ERROR, (t), (tok), (fmt))

TARE_DEF void debug_print_token(const Tokenizer *t, const Token *tok);

TARE_DEF bool tok_sv_cmp(Token *tok, StringView sv);
TARE_DEF bool tok_eq(Token *t1, Token *t2);

TARE_DEF bool to_token(Tokenizer *t, size_t index);
TARE_DEF bool first_token(Tokenizer *t);
TARE_DEF bool next_token(Tokenizer *t);
TARE_DEF bool prev_token(Tokenizer *t);
TARE_DEF Token peek_prev_token(const Tokenizer *t);
TARE_DEF Token peek_next_token(const Tokenizer *t);
TARE_DEF Token peek_forward_token(const Tokenizer *t, size_t forward);

TARE_DEF bool expect_token_type(Tokenizer *t, TokenType type);
TARE_DEF bool expect_token_type_two(Tokenizer *t, TokenType a, TokenType b);
TARE_DEF bool expect_token_type_three(Tokenizer *t, TokenType a, TokenType b, TokenType c);
TARE_DEF bool expect_name(Tokenizer *t);
TARE_DEF bool expect_whole_num(Tokenizer *t);
TARE_DEF bool expect_frac_num(Tokenizer *t);
TARE_DEF bool expect_keyword(Tokenizer *t);
TARE_DEF bool expect_special(Tokenizer *t, SpecialType s);
TARE_DEF bool expect_special_many_function(Tokenizer *t, ...);
TARE_DEF bool expect_special_sequence_function(Tokenizer *t, ...);
TARE_DEF bool expect_char(Tokenizer *t);
TARE_DEF bool expect_vid(Tokenizer *t);
TARE_DEF bool expect_tid(Tokenizer *t);
TARE_DEF bool expect_fid(Tokenizer *t);

TARE_DEF bool expect_num_or_tape(Tokenizer *t);
TARE_DEF bool expect_vid_or_tape(Tokenizer *t);
TARE_DEF bool expect_num_or_vid_or_tape(Tokenizer *t);
TARE_DEF bool expect_tid_or_const(Tokenizer *t);

#define expect_special_many(t, ...)                             \
  expect_special_many_function((t), __VA_ARGS__, SPECIAL_TYPES)

#define expect_special_sequence(t, ...)                                 \
  expect_special_sequence_function((t), __VA_ARGS__, SPECIAL_TYPES)

#endif // TOKENIZER_H_

#ifdef TOKENIZER_IMPLEMENTATION

static_assert(SPECIAL_TYPES == 23, "Amount of special characters has changed. Please update the `special_index` function to properly account for that.");
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
  if (!read_file(path, &t->l.buf)) return false;
  return fill_tokenizer(t); // Tokenize the text.
}

TARE_DEF bool fill_tokenizer(Tokenizer *t) {
  if (t == NULL || t->l.buf.items == NULL) return false; // Sanity check.
  first_char(&t->l); // Just in case.
  do { // Do while cause it fits this use case.
    if (isspace(t->l.c)) continue; // Skip whitespace.
    
    // Token still mostly zero-initialized, but with some fields
    // filled in to avoid needless repetition.
    Token tok = {.f = t->l.buf.items + t->l.i, .l = 1, .c = t->l.c,
                 .row = t->l.row + 1, .col = t->l.col + 1,
                 .s = special_index(t->l.c)};

    // Try to look ahead fo check for an indication of a fractional
    // number. May expand to look for other indications of a number.
    bool is_frac = is_digit(peek_next_char(&t->l)) && tok.s == DOT;
    if (tok.s == DQUOTE) { // Handle string literal.
      tok.t = TOKEN_TYPE_STRING;
      // First character pointed at the dqoute, but should point right
      // after it. Similarly, the length, the first character, the
      // row, and the col, should also reflect that fact.
      tok.f++, tok.l--;
      if (!lexer_advance_char_forward(&t->l)) return false;
      tok.c = t->l.c, tok.row = t->l.row + 1, tok.col = t->l.col + 1;
      while (t->l.c != Specials[DQUOTE]) { // Handle the rest of the string.
        tok.l++;
        if (!lexer_advance_char_forward(&t->l)) break;
        // Skip escaped charcters to not end the string too early.
        if (t->l.c == Specials[ESC]) {
          if (!lexer_advance_char_forward(&t->l)) break;
          tok.l++;
          if (!lexer_advance_char_forward(&t->l)) break;
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
      if (!lexer_advance_char_forward(&t->l)) return false;
      tok.c = t->l.c, tok.row = t->l.row + 1, tok.col = t->l.col + 1;
      if (t->l.c == Specials[ESC]) { // Check for escaped characters.
        if (!lexer_advance_char_forward(&t->l)) return false;
        tok.l++; // Increase length to capture the whole thing.
      }
      if (!lexer_advance_char_forward(&t->l)) return false;
      if (t->l.c != Specials[SQUOTE]) return false;
      da_append(t, tok);
      continue;
    } else if (tok.s < SPECIAL_TYPES && !is_frac) {
      // Handle special characters.
      if (tok.s == DIV) { // Check for comments
        SpecialType next = special_index(peek_next_char(&t->l));
        if (next == DIV) { // Single-line comment.
          while (next_char(&t->l)) if (t->l.c == '\n') break;
          continue;
        } else if (next == MULT) { // Multi-line comment.
          while (lexer_advance_char_forward(&t->l)) {
            SpecialType prev = special_index(peek_prev_char(&t->l));
            if (prev == MULT && special_index(t->l.c) == DIV) break;
          }
          continue;
        }
      }
      tok.t = TOKEN_TYPE_SPECIAL;
      da_append(t, tok);
      continue;
    }

    // Numeric or name token.
    bool is_whole = is_digit(tok.c); // Prepare for numeric token.
    double frac = 0;
    double frac_mult = 1;
    if (is_whole) tok.u64 = tok.u64 * 10 + (tok.c - '0');
    while (special_index(peek_next_char(&t->l)) >= SPECIAL_TYPES) {
      // Tokenize the rest of the token.
      if (!lexer_advance_char_forward(&t->l)) break;
      if (isspace(t->l.c)) break;
      tok.l++;
      is_whole &= is_digit(t->l.c);
      if (is_whole) tok.u64 = tok.u64 * 10 + (t->l.c - '0');
      if (is_whole && special_index(peek_next_char(&t->l)) == DOT) {
        if (is_frac) return false;
        is_frac = true;
        is_whole = false;
        if (!lexer_advance_char_forward(&t->l)) break; // Skip the dot.
        tok.l++;
        if (!lexer_advance_char_forward(&t->l)) break; // Digit after the dot.
        tok.l++;
      }
      if (is_frac) {
        frac = frac * 10 + t->l.c - '0';
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
  } while (lexer_advance_char_forward(&t->l));

  return true;
}

TARE_DEF void print_loc(FILE *stream, const Tokenizer *t, const Token *tok) {
  fprintf(stream, "%s:%zu:%zu: ", t->path, tok->row, tok->col);
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
  case TOKEN_TYPE_SPECIAL: printf("(special) %c\n", tok->c); break;
  case TOKEN_TYPE_STRING: printf("(string) %.*s\n", TOK_ARG(tok)); break;
  case TOKEN_TYPE_CHAR: printf("(char) %.*s\n", TOK_ARG(tok)); break;
  case TOKEN_TYPE_VID:
    printf("(vid) %.*s (%zu)\n", TOK_ARG(tok), tok->vid); break;
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

TARE_DEF bool expect_vid(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_VID);
}

TARE_DEF bool expect_tid(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_TID);
}

TARE_DEF bool expect_fid(Tokenizer *t) {
  return expect_token_type(t, TOKEN_TYPE_FID);
}

TARE_DEF bool expect_num_or_tape(Tokenizer *t) {
  if (!expect_token_type_two(t, TOKEN_TYPE_WHOLE_NUM, TOKEN_TYPE_KEYWORD))
    return false;
  if (t->t->t == TOKEN_TYPE_WHOLE_NUM) return true;
  if (t->t->k == KEY_TAPE || t->t->k == KEY_HEAD ||
      t->t->k == KEY_BASE || t->t->k == KEY_INDEX) return true;
  diag_errf(t, t->t, "expected whole number or keyword `%.*s`, `%.*s`, `%.*s`, or `%.*s`, but got %s instead\n",
          SV_ARG(Keywords[KEY_TAPE]), SV_ARG(Keywords[KEY_HEAD]),
          SV_ARG(Keywords[KEY_BASE]), SV_ARG(Keywords[KEY_INDEX]),
          TokenTypeNames[t->t->t]);
  return false;
}

TARE_DEF bool expect_vid_or_tape(Tokenizer *t) {
  if (!expect_token_type_two(t, TOKEN_TYPE_VID, TOKEN_TYPE_KEYWORD))
    return false;
  if (t->t->t == TOKEN_TYPE_VID) return true;
  if (t->t->k == KEY_TAPE || t->t->k == KEY_HEAD ||
      t->t->k == KEY_BASE || t->t->k == KEY_INDEX) return true;
  diag_errf(t, t->t, "expected variable identifier or keyword `%.*s`, `%.*s`, `%.*s`, or `%.*s`, but got %s instead\n",
          SV_ARG(Keywords[KEY_TAPE]), SV_ARG(Keywords[KEY_HEAD]),
          SV_ARG(Keywords[KEY_BASE]), SV_ARG(Keywords[KEY_INDEX]),
          TokenTypeNames[t->t->t]);
  return false;
}

TARE_DEF bool expect_num_or_vid_or_tape(Tokenizer *t) {
  if (!expect_token_type_three(t, TOKEN_TYPE_WHOLE_NUM, TOKEN_TYPE_VID,
                               TOKEN_TYPE_KEYWORD)) return false;
  if (t->t->t == TOKEN_TYPE_WHOLE_NUM) return true;
  else if (t->t->t == TOKEN_TYPE_VID) return true;
  
  if (t->t->k == KEY_TAPE || t->t->k == KEY_HEAD ||
      t->t->k == KEY_BASE || t->t->k == KEY_INDEX) return true;
  diag_errf(t, t->t, "expected whole number, variable identifier, or keyword `%.*s`, `%.*s`, `%.*s`, or `%.*s`, but got %s instead\n",
          SV_ARG(Keywords[KEY_TAPE]), SV_ARG(Keywords[KEY_HEAD]),
          SV_ARG(Keywords[KEY_BASE]), SV_ARG(Keywords[KEY_INDEX]),
          TokenTypeNames[t->t->t]);
  return false;
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

