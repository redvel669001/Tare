#define CMD_IMPLEMENTATION
#include "cmd.h"

#define unimpl(name)                                    \
  do {                                                  \
    printf("\n------------------------------\n");       \
    printf("%s:%d: %s is unimplemented!\n",             \
           __FILE__, __LINE__, (name));                 \
    debug_print_token(t, t->t);                         \
    printf("------------------------------\n\n");       \
  } while (0)

TARE_DEF bool check_bounds(size_t index, size_t count);

#define STR_IMPLEMENTATION
#include "str.h"

#define LEXER_IMPLEMENTATION
#include "lexer.h"

#define TOKENIZER_IMPLEMENTATION
#include "tokenizer.h"

#define PARSER_IMPLEMENTATION
#include "parser.h"

#define GENERATOR_IMPLEMENTATION
#include "generator.h"

#define FLAGS_IMPLEMENTATION
#include "flags.h"

/* #define SIMULATOR_IMPLEMENTATION */
/* #include "simulator.h" */

typedef struct {
  const char *path;
  const char *output;
  const char *fasm_input;
  const char *output_bin;
  String arena;
  StringView src;
} Paths;

TARE_DEF size_t find_final_dot(const char *str, size_t len);
TARE_DEF bool paths_from_tare(const char *p, Paths *ps, StringView build);
TARE_DEF void str_to_tare_template(const char *string); // Testing thing

int main(int argc, char **argv) {
  StringView src = SV_MAKE(./examples/);
  String prog = {0};

  if (argc < 2) {
    fprintf(stderr, "error: Input `tare` file not provided!\n");
    return 1;
  }

  argc--;
  argv++;
  
  const char *path_plain = *argv;

  Flags flags = {0};
  init_flags(&flags);
  parse_flags(argc, (const char**) argv, &flags);
  
  size_t path_len = strlen(path_plain);
  if (!append_sv_to_string(&prog, src)) return 1;
  if (!append_to_string(&prog, path_plain, path_len)) return 1;
  da_append(&prog, 0);
  const char *path = path_plain;

  Paths paths = {.src = src};
  StringView build = SV_MAKE(build/);
  if (!paths_from_tare(path, &paths, build)) return 1;
  
  Tokenizer t = {0};
  if (!tokenize_file(paths.path, &t)) return 1;

  // Simulator is currently deprecated
  if (flags.items[FLAG_SIMULATE].on) {
    fprintf(stderr, "Simulation mode isn't supported\n");
    /* if (!sim_tare(&t)) return 1; */
    return 0;
  }
  
  Functions funcs = {0};
  Longs gotos = {0};
  Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos};
  if (!parse_file(&p)) return 1;
  if (!gen_fasm(&p, paths.output)) return 1;
  
  Cmd cmd = {0};
  int fdout = fileno(stdout);
  Redirect redirect = {0};
  close(fdout);
  
  cmd_append(&cmd, "fasm", paths.fasm_input);
  if (!run_cmd(&cmd, redirect, true)) return 1;

  cmd_append(&cmd, "chmod", "+x", paths.output_bin);
  if (!run_cmd(&cmd, redirect, true)) return 1;

  if (t.l.items) free(t.l.items);
  if (t.items) free(t.items);
  if (cmd.items) free(cmd.items);
  if (flags.items) free(flags.items);
  if (paths.arena.items) free(paths.arena.items);
  if (prog.items) free(prog.items);
  
  return 0;
}

TARE_DEF bool check_bounds(size_t index, size_t count) {
  return index < count;
}

TARE_DEF size_t find_final_dot(const char *str, size_t len) {
  size_t result = 0;
  for (size_t i = 0; i < len; i++) if (str[i] == '.') result = i;
  return result;
}

TARE_DEF bool paths_from_tare(const char *p, Paths *ps, StringView build) {
  ps->path = p;
  const char *path = p + ps->src.l;
  size_t final_dot = find_final_dot(p, strlen(p)) - ps->src.l;
  
  if (final_dot == 0) return false;
  if (!append_to_string(&ps->arena, "./", 2)) return false;
  ps->fasm_input = ps->arena.items + ps->arena.count;
  ps->output = ps->arena.items + ps->arena.count;
  if (!append_to_string(&ps->arena, build.s, build.l)) return false;
  if (!append_to_string(&ps->arena, path, final_dot)) return false;
  if (!append_to_string(&ps->arena, ".s", 2)) return false;
  da_append(&ps->arena, 0);
  
  ps->output_bin = ps->arena.items + ps->arena.count;
  if (!append_to_string(&ps->arena, "./", 2)) return false;
  if (!append_to_string(&ps->arena, build.s, build.l)) return false;
  if (!append_to_string(&ps->arena, path, final_dot)) return false;
  da_append(&ps->arena, 0);

  return true;
}

// Testing thing.
TARE_DEF void str_to_tare_template(const char *string) {
  size_t len = strlen(string);
  printf("len = %zu\n", len);
  
  const char *test_str = malloc(len);
  for (size_t i = 0; i < len;) {
    if (i + 8 <= len) {
      size_t test = *(size_t*)(string + i);
      *(size_t*)(test_str + i) = test;
      printf("test%zu (64 bits) = %zu\n", i, test);
      i += 8;
    } else if (i + 4 <= len) {
      unsigned int test = *(unsigned int*)(string + i);
      *(unsigned int*)(test_str + i) = test;
      printf("test%zu (32 bits) = %u\n", i, test);
      i += 4;
    } else if (i + 2 <= len) {
      unsigned short test = *(unsigned short*)(string + i);
      *(unsigned short*)(test_str + i) = test;
      printf("test%zu (16 bits) = %u\n", i, test);
      i += 2;
    } else if (i + 1 <= len) {
      unsigned char test = *(unsigned char*)(string + i);
      *(unsigned char*)(test_str + i) = test;
      printf("test%zu (8 bits) = %u\n", i, test);
      i += 1;
    }
  }
}
