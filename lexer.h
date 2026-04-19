#ifndef LEXER_H_
#define LEXER_H_

#include <ctype.h>

typedef struct {
  char *items;
  size_t count, capacity;
  
  size_t index;
  char *c;
} Lexer;

TARE_DEF bool read_file_to_lexer(const char *path, Lexer *l);

TARE_DEF bool is_digit(char c);

TARE_DEF bool to_char(Lexer *l, size_t index);
TARE_DEF bool first_char(Lexer *l);
TARE_DEF bool next_char(Lexer *l);
TARE_DEF bool prev_char(Lexer *l);
TARE_DEF char peek_next_char(const Lexer *l);
TARE_DEF char peek_prev_char(const Lexer *l);

#endif // LEXER_H_

#ifdef LEXER_IMPLEMENTATION

TARE_DEF bool read_file_to_lexer(const char *path, Lexer *l) {
  String str = {0};
  if (!read_file(path, &str)) return false;
  l->items = str.items;
  l->count = str.count;
  l->capacity = str.capacity;
  return true;
}

TARE_DEF bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

TARE_DEF bool to_char(Lexer *l, size_t index) {
  if (!check_bounds(index, l->count)) return false;
  l->c = l->items + index;
  l->index = index;
  return true;
}

TARE_DEF bool first_char(Lexer *l) {
  return to_char(l, 0);
}

TARE_DEF bool next_char(Lexer *l) {
  return to_char(l, l->index + 1);
}

TARE_DEF bool prev_char(Lexer *l) {
  return to_char(l, l->index - 1);
}

TARE_DEF char peek_next_char(const Lexer *l) {
  if (!check_bounds(l->index + 1, l->count)) return 0;
  return l->items[l->index + 1];
}

TARE_DEF char peek_prev_char(const Lexer *l) {
  if (!check_bounds(l->index - 1, l->count)) return 0;
  return l->items[l->index - 1];
}

#endif // LEXER_IMPLEMENTATION
