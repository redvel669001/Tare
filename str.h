#ifndef STR_H_
#define STR_H_

#include <stdarg.h>

// Lots of things in this header file were copied from nob.h, though
// most of these things were additionally simplified.
// https://github.com/tsoding/nob.h/blob/main/nob.h

#define SV_ARG(sv) (int) (sv).l, (sv).s
#define SV_MAKE(string) {.s = #string, .l = sizeof((#string)) - 1}
#define sv_from_cstr(cstr) {.s = (cstr), .l = strlen((cstr))}

typedef struct {
  const char *s;
  size_t l;
} StringView;

typedef struct {
  char *items;
  size_t count;
  size_t capacity;
} String;

TARE_DEF bool read_file(const char *path, String *buf);

TARE_DEF void str_appendf(String *str, const char *fmt, ...);
TARE_DEF bool append_to_string(String *str, const char *string, size_t len);
TARE_DEF bool append_sv_to_string(String *str, StringView sv);
TARE_DEF bool append_cstr_to_string(String *str, const char *cstr);

TARE_DEF bool sv_eq(StringView sv1, StringView sv2);

#endif // STR_H_

#ifdef STR_IMPLEMENTATION
TARE_DEF bool read_file(const char *path, String *buf) {
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    fprintf(stderr, "error: Could not open file `%s`.\n", path);
    return false;
  }

  if (fseek(f, 0, SEEK_END) != 0) {
    fprintf(stderr, "error: Could not seek to the end of file `%s`.\n", path);
    fclose(f);
    return false;
  }
  
  long file_size = ftell(f);
  if (file_size == -1) {
    fprintf(stderr, "error: Could not measure size of file `%s`.\n", path);
    fclose(f);
    return false;
  }

  buf->items = malloc(file_size);
  if (buf->items == NULL) {
    fprintf(stderr, "error: Malloc failed!\n");
    fclose(f);
    return false;
  }
  
  buf->capacity = file_size;
  fseek(f, 0, SEEK_SET);
  if (fread(buf->items, file_size, 1, f) == 0) {
    fprintf(stderr, "error: Could not read file `%s`. It there was an error, or the file was empty.\n", path);
  }
  buf->count = buf->capacity;
  
  fclose(f);
  return true;
}

TARE_DEF void str_appendf(String *str, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  size_t n = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  da_reserve(str, str->count + n + 1);
  va_start(args, fmt);
  vsnprintf(str->items + str->count, n + 1, fmt, args);
  va_end(args);

  str->count += n;
}

TARE_DEF bool append_to_string(String *str, const char *string, size_t len) {
  if (str->count + len > str->capacity) {
    if (str->capacity == 0) {
      str->capacity = DA_INIT_CAPACITY;
    }
    while (str->count + len > str->capacity) {
      str->capacity *= 2;
    }
    if (str->items == NULL) str->items = malloc(str->capacity);
    else str->items = realloc(str->items, str->capacity);
  }

  memcpy(str->items + str->count, string, len);
  str->count += len;

  return str->items != NULL;
}

TARE_DEF bool append_sv_to_string(String *str, StringView sv) {
  return append_to_string(str, sv.s, sv.l);
}

TARE_DEF bool append_cstr_to_string(String *str, const char *cstr) {
  return append_to_string(str, cstr, strlen(cstr));
}

TARE_DEF bool sv_eq(StringView sv1, StringView sv2) {
  if (sv1.l != sv2.l) return false;
  return strncmp(sv1.s, sv2.s, sv1.l) == 0;
}

#endif // STR_IMPLEMENTATION
