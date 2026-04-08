#ifndef LEXER_H_
#define LEXER_H_

#include <ctype.h>

typedef struct {
  String buf;
  size_t i; // index
  
  size_t row;
  size_t col;
  
  char c;
  
  /* const char *f; */
  /* size_t l; */
} Lexer;

TARE_DEF bool is_digit(char c);

TARE_DEF bool to_char(Lexer *l, size_t index);
TARE_DEF bool first_char(Lexer *l);
TARE_DEF bool next_char(Lexer *l);
TARE_DEF bool prev_char(Lexer *l);
TARE_DEF char peek_next_char(const Lexer *l);
TARE_DEF char peek_prev_char(const Lexer *l);

TARE_DEF bool lexer_advance_char_forward(Lexer *l);

#endif // LEXER_H_

#ifdef LEXER_IMPLEMENTATION

TARE_DEF bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

TARE_DEF bool to_char(Lexer *l, size_t index) {
  if (!check_bounds(index, l->buf.count)) return false;
  l->c = l->buf.items[index];
  l->i = index;
  return true;
}

TARE_DEF bool first_char(Lexer *l) {
  return to_char(l, 0);
}

TARE_DEF bool next_char(Lexer *l) {
  return to_char(l, l->i + 1);
}

TARE_DEF bool prev_char(Lexer *l) {
  return to_char(l, l->i - 1);
}

TARE_DEF char peek_next_char(const Lexer *l) {
  if (!check_bounds(l->i + 1, l->buf.count)) return 0;
  return l->buf.items[l->i + 1];
}

TARE_DEF char peek_prev_char(const Lexer *l) {
  if (!check_bounds(l->i - 1, l->buf.count)) return 0;
  return l->buf.items[l->i - 1];
}

TARE_DEF bool lexer_advance_char_forward(Lexer *l) {
  if (!next_char(l)) return false;
  l->col++;
  if (peek_prev_char(l) == '\n') {
    l->row++;
    l->col = 0;
  }
  return true;
}

#endif // LEXER_IMPLEMENTATION
