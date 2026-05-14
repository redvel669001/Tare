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

#define CODEGEN_IMPLEMENTATION
#include "codegen.h"

#define FLAGS_IMPLEMENTATION
#include "flags.h"

/* #define SIMULATOR_IMPLEMENTATION */
/* #include "simulator.h" */

#include <time.h>

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

TARE_DEF size_t get_current_time(void);
TARE_DEF void print_elapsed_time(size_t start, size_t end, const char *subject);

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

  bool time_tokenization = false, time_parsing = false, time_codegen = false;

#ifdef THOROUGH_TIMING
  double tokenization_time_total = 0;
  double tokenization_time_best = 1;
  double tokenization_time_worst = 0;
  
  double parsing_time_total = 0;
  double parsing_time_best = 1;
  double parsing_time_worst = 0;
  
  double codegen_time_total = 0;
  double codegen_time_best = 1;
  double codegen_time_worst = 0;
  
  #define SAMPLE_SIZE 10000
  for (unsigned int counter = 0; counter < SAMPLE_SIZE; counter++) {

    double current_action_time = 0;
#endif // THOROUGH_TIMING
    Tokenizer t = {0};
    size_t start = get_current_time();
    if (!tokenize_file(paths.path, &t)) return 1;
    size_t end = get_current_time();
#ifdef THOROUGH_TIMING
    if (time_tokenization) {
      current_action_time = ((double) end - start) / 1000000000;
      
      tokenization_time_total += current_action_time;
      if (current_action_time > tokenization_time_worst) tokenization_time_worst = current_action_time;
      if (current_action_time < tokenization_time_best) tokenization_time_best = current_action_time;
    }
#else
    if (time_tokenization) print_elapsed_time(start, end, "Tokenization");
#endif // THOROUGH_TIMING

    // Simulator is currently deprecated
    if (flags.items[FLAG_SIMULATE].on) {
      fprintf(stderr, "Simulation mode isn't supported\n");
      /* if (!sim_tare(&t)) return 1; */
      return 0;
    }
  
    Functions funcs = {0};
    Longs gotos = {0};
    Vars globals = {0};
    Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
    if (time_parsing) start = get_current_time();
    if (!parse_file(&p)) return 1;
    if (time_parsing) end = get_current_time();
#ifdef THOROUGH_TIMING
    if (time_parsing) {
      current_action_time = ((double) end - start) / 1000000000;
      
      parsing_time_total += current_action_time;
      if (current_action_time > parsing_time_worst) parsing_time_worst = current_action_time;
      if (current_action_time < parsing_time_best) parsing_time_best = current_action_time;
    }
#else
    if (time_parsing) print_elapsed_time(start, end, "Parsing");
#endif // THOROUGH_TIMING

    if (time_codegen) start = get_current_time();
    if (!gen_fasm(&p, paths.output)) return 1;
    if (time_codegen) end = get_current_time();
#ifdef THOROUGH_TIMING
    if (time_codegen) {
      current_action_time = ((double) end - start) / 1000000000;
      
      codegen_time_total += current_action_time;
      if (current_action_time > codegen_time_worst) codegen_time_worst = current_action_time;
      if (current_action_time < codegen_time_best) codegen_time_best = current_action_time;
    }
#else
    if (time_codegen) print_elapsed_time(start, end, "Codegen");
#endif // THOROUGH_TIMING

#ifndef THOROUGH_TIMING
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
  
    for (size_t i = 0 ; i < funcs.count; i++) {
      Function fn = funcs.items[i];
      if (fn.items) free(fn.items);
      if (fn.lvars.items) free(fn.lvars.items);
      if (fn.args.items) free(fn.args.items);
      if (fn.rets.items) free(fn.rets.items);
    }

    if (p.stack.items) free(p.stack.items);

    if (funcs.items) free(funcs.items);

    if (gotos.items) free(gotos.items);
#else
  }

  double tokenization_time_average = tokenization_time_total / SAMPLE_SIZE;
  double parsing_time_average = parsing_time_total / SAMPLE_SIZE;
  double codegen_time_average = codegen_time_total / SAMPLE_SIZE;

  printf("Tokenization:\n");
  printf("    Total: %lf\n", tokenization_time_total);
  printf("    Average: %lf\n", tokenization_time_average);
  printf("    Best: %lf\n", tokenization_time_best);
  printf("    Worst: %lf\n", tokenization_time_worst);
  
  printf("Parsing:\n");
  printf("    Total: %lf\n", parsing_time_total);
  printf("    Average: %lf\n", parsing_time_average);
  printf("    Best: %lf\n", parsing_time_best);
  printf("    Worst: %lf\n", parsing_time_worst);

  printf("Codegen:\n");
  printf("    Total: %lf\n", codegen_time_total);
  printf("    Average: %lf\n", codegen_time_average);
  printf("    Best: %lf\n", codegen_time_best);
  printf("    Worst: %lf\n", codegen_time_worst);
  
#endif // THOROUGH_TIMING
  
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

TARE_DEF size_t get_current_time(void) {
  static struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (size_t)ts.tv_sec * 1000000000LL + (size_t)ts.tv_nsec;
}

TARE_DEF void print_elapsed_time(size_t start, size_t end, const char *subject) {
  printf("\n%s: %lf\n", subject, ((double) end - start) / 1000000000);
}
