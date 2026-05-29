/* #define THOROUGH_TIMING */

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

#define SIMULATOR_IMPLEMENTATION
#include "simulator.h"

#include <time.h>

typedef struct {
  const char *path;
  const char *output;
  const char *fasm_input;
  const char *output_bin;
  String arena;
  StringView src;
} Paths;

typedef struct {
  double total;
  double best;
  double worst;
} ThoroughTimer;

TARE_DEF size_t find_final_dot(const char *str, size_t len);
TARE_DEF bool paths_from_tare(const char *p, Paths *ps, StringView build);
TARE_DEF void str_to_tare_template(const char *string); // Testing thing

TARE_DEF size_t get_current_time(void);
TARE_DEF void print_elapsed_time(size_t start, size_t end, const char *subject);

int main(int argc, char **argv) {
  int ret = 0;

  StringView src = SV_MAKE(./examples/);
  String prog = {0};

  const char *program = *argv;

  argc--;
  argv++;
  
  const char *path_plain = *argv;

  enum {
    FLAG_SIMULATE = 0,
    FLAG_COMPILE_FASM,
    FLAG_RUN,
    FLAG_TIME_TOKENIZER,
    FLAG_TIME_PARSER,
    FLAG_TIME_CODEGEN,
    FLAG_TIME_FASM,
    FLAG_HELP,
    FLAGS_COUNT,
  };

  static_assert(FLAGS_COUNT == 8, "Flags count has been chagned. Please update the flags to match the new count.");
  
  Flag sim = {
    .name_short = "-s", .name_long = "--simulate",
    .description = "invoke an interpreter instead of a compiler.",
  };
  Flag comp = {
    .name_short = "-c", .name_long = "--compile",
    .description = "compile a native executable.",
  };
  Flag run = {
    .name_short = "-r", .name_long = "--run",
    .description = "compile and run a native executable.",
  };
  Flag time_tokenization = {
    .name_short = "-tt", .name_long = "--time-tokenization",
    .description = "calculate the time for tokenization.",
  };
  Flag time_parsing = {
    .name_short = "-tp", .name_long = "--time-parsing",
    .description = "calculate the time for parsing.",
  };
  Flag time_codegen = {
    .name_short = "-tc", .name_long = "--time-codegen",
    .description = "calculate the time for code generation.",
  };
  Flag time_fasm = {
    .name_short = "-tf", .name_long = "--time-fasm",
    .description = "calculate the time for the assembler (fasm).",
  };
  Flag help = {
    .name_short = "-h", .name_long = "--help",
    .description = "present this infromation.",
  };

  static_assert(FLAGS_COUNT == 8, "Flags count has been chagned. Please update the array to match the new count.");
  Flag *flag_array[FLAGS_COUNT] = {
    [FLAG_SIMULATE] = &sim,
    [FLAG_COMPILE_FASM] = &comp,
    [FLAG_RUN] = &run,
    [FLAG_TIME_TOKENIZER] = &time_tokenization,
    [FLAG_TIME_PARSER] = &time_parsing,
    [FLAG_TIME_CODEGEN] = &time_codegen,
    [FLAG_TIME_FASM] = &time_fasm,
    [FLAG_HELP] = &help,
  };

  Flags flags = { .items = flag_array, .count = FLAGS_COUNT, };
  parse_flags(argc, (const char**) argv, &flags);

  if (argc < 1) {
    fprintf(stderr, "Error: input `tare` file not provided!\n");
    print_usage(stderr, program, "<input> ", flags);
    return 1;
  }
  
  if (help.on) {
    print_usage(stdout, program, "<input> ", flags);
    return 0;
  }

  if (run.on) comp.on = true;
  if (comp.on) sim.on = false;

  size_t path_len = strlen(path_plain);
  if (!append_sv_to_string(&prog, src)) return 1;
  if (!append_to_string(&prog, path_plain, path_len)) return 1;
  da_append(&prog, 0);
  const char *path = path_plain;

  Paths paths = {.src = src};
  StringView build = SV_MAKE(build/);
  if (!paths_from_tare(path, &paths, build)) return 1;

#ifdef THOROUGH_TIMING
  ThoroughTimer tokenization_timer = { .best = 1 };
  ThoroughTimer parsing_timer      = { .best = 1 };
  ThoroughTimer codegen_timer      = { .best = 1 };
  ThoroughTimer fasm_timer         = { .best = 1 };
  
  #define SAMPLE_SIZE 10000
  for (unsigned int counter = 0; counter < SAMPLE_SIZE; counter++) {

    double current_action_time = 0;
#endif // THOROUGH_TIMING
    Tokenizer t = {0};
    size_t start = get_current_time();
    if (!tokenize_file(paths.path, &t)) return 1;
    size_t end = get_current_time();
#ifdef THOROUGH_TIMING
    if (time_tokenization.on) {
      current_action_time = ((double) end - start) / 1000000000;
      
      tokenization_timer.total += current_action_time;
      if (current_action_time > tokenization_timer.worst) tokenization_timer.worst = current_action_time;
      if (current_action_time < tokenization_timer.best) tokenization_timer.best = current_action_time;
    }
#else
    if (time_tokenization.on) print_elapsed_time(start, end, "Tokenization");
#endif // THOROUGH_TIMING
  
    Functions funcs = {0};
    Longs gotos = {0};
    Vars globals = {0};
    Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
    if (time_parsing.on) start = get_current_time();
    if (!parse_file(&p)) return 1;
    if (time_parsing.on) end = get_current_time();
#ifdef THOROUGH_TIMING
    if (time_parsing.on) {
      current_action_time = ((double) end - start) / 1000000000;
      
      parsing_timer.total += current_action_time;
      if (current_action_time > parsing_timer.worst) parsing_timer.worst = current_action_time;
      if (current_action_time < parsing_timer.best) parsing_timer.best = current_action_time;
    }
#else
    if (time_parsing.on) print_elapsed_time(start, end, "Parsing");
#endif // THOROUGH_TIMING

    Cmd cmd = {0};

    if (sim.on) ret = sim_tare(&p);
    else {

      if (time_codegen.on) start = get_current_time();
      if (!gen_fasm(&p, paths.output)) return 1;
      if (time_codegen.on) end = get_current_time();
#ifdef THOROUGH_TIMING
      if (time_codegen.on) {
        current_action_time = ((double) end - start) / 1000000000;
      
        codegen_timer.total += current_action_time;
        if (current_action_time > codegen_timer.worst) codegen_timer.worst = current_action_time;
        if (current_action_time < codegen_timer.best) codegen_timer.best = current_action_time;
      }
#else
      if (time_codegen.on) print_elapsed_time(start, end, "Codegen");
#endif // THOROUGH_TIMING

      /* #ifndef THOROUGH_TIMING */
      /* int fdout = fileno(stdout); */
      /* Redirect redirect = {0}; */
      /* close(fdout); */

      FILE *logs = fopen("logs.log", "w");
      int logs_fd = fileno(logs);
      Redirect redirect = {.fdout = &logs_fd};
#ifdef THOROUGH_TIMING
      bool display = false;
#else
      bool display = true;
#endif // THOROUGH_TIMING

      if (time_fasm.on) start = get_current_time();
      cmd_append(&cmd, "fasm", paths.fasm_input);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (time_fasm.on) end = get_current_time();

      if (logs) fclose(logs);
#ifdef THOROUGH_TIMING
      if (time_fasm.on) {
        current_action_time = ((double) end - start) / 1000000000;
      
        fasm_timer.total += current_action_time;
        if (current_action_time > fasm_timer.worst) fasm_timer.worst = current_action_time;
        if (current_action_time < fasm_timer.best) fasm_timer.best = current_action_time;
      }
#else
      if (time_fasm.on) print_elapsed_time(start, end, "Fasm");
#endif // THOROUGH_TIMING

      redirect.fdout = NULL;
      cmd_append(&cmd, "chmod", "+x", paths.output_bin);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (run.on) {
        cmd_append(&cmd, paths.output_bin);
        if (!run_cmd(&cmd, redirect, display)) return 1;
      }
    }
/* #endif // THOROUGH_TIMING */
    if (t.l.items) free(t.l.items);
    if (t.items) free(t.items);
#ifndef THOROUGH_TIMING
    if (cmd.items) free(cmd.items);
    /* if (flags.items) free(flags.items); */
    if (paths.arena.items) free(paths.arena.items);
    if (prog.items) free(prog.items);
#endif // THOROUGH_TIMING
  
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
    if (globals.items) free(globals.items);
#ifndef THOROUGH_TIMING
#else
  }

  double tokenization_time_average = tokenization_timer.total / SAMPLE_SIZE;
  double parsing_time_average = parsing_timer.total / SAMPLE_SIZE;
  double codegen_time_average = codegen_timer.total / SAMPLE_SIZE;
  double fasm_time_average = fasm_timer.total / SAMPLE_SIZE;

  printf("Tokenization:\n");
  printf("    Total: %lf\n", tokenization_timer.total);
  printf("    Average: %lf\n", tokenization_time_average);
  printf("    Best: %lf\n", tokenization_timer.best);
  printf("    Worst: %lf\n", tokenization_timer.worst);
  
  printf("Parsing:\n");
  printf("    Total: %lf\n", parsing_timer.total);
  printf("    Average: %lf\n", parsing_time_average);
  printf("    Best: %lf\n", parsing_timer.best);
  printf("    Worst: %lf\n", parsing_timer.worst);

  printf("Codegen:\n");
  printf("    Total: %lf\n", codegen_timer.total);
  printf("    Average: %lf\n", codegen_time_average);
  printf("    Best: %lf\n", codegen_timer.best);
  printf("    Worst: %lf\n", codegen_timer.worst);

  printf("Fasm:\n");
  printf("    Total: %lf\n", fasm_timer.total);
  printf("    Average: %lf\n", fasm_time_average);
  printf("    Best: %lf\n", fasm_timer.best);
  printf("    Worst: %lf\n", fasm_timer.worst);
  
#endif // THOROUGH_TIMING
  
  return ret;
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
