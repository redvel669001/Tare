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

#define TAPE_SIZE (size_t) 1024
#define ARG_TAPE_SIZE (size_t) 1024
#define RET_TAPE_SIZE (size_t) 1024
#define GLOBAL_VAR_TAPE_SIZE (size_t) 1024
#define LOCAL_VAR_TAPE_SIZE (size_t) 1024

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

TARE_DEF int comp_or_sim_multiple_times(Paths paths, bool time_tokenization, bool time_parsing, bool time_codegen, bool time_fasm, bool time_simulator, bool run, bool sim, size_t tape_size, size_t arg_tape_size, size_t ret_tape_size, size_t global_var_tape_size, size_t local_var_tape_size, size_t count);
TARE_DEF int comp_or_sim_once(Paths paths, bool time_tokenization, bool time_parsing, bool time_codegen, bool time_fasm, bool time_simulator, bool run, bool sim, size_t tape_size, size_t arg_tape_size, size_t ret_tape_size, size_t global_var_tape_size, size_t local_var_tape_size, String *prog);
TARE_DEF void time_current_action(ThoroughTimer *t, size_t start, size_t end);

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
    FLAG_TIME_SIMULATOR,
    FLAG_TIME_THOROUGHLY,
    FLAG_TAPE_SIZE,
    FLAG_ARG_TAPE_SIZE,
    FLAG_RET_TAPE_SIZE,
    FLAG_GLOBAL_VAR_TAPE_SIZE,
    FLAG_LOCAL_VAR_TAPE_SIZE,
    FLAG_HELP,
    FLAGS_COUNT,
  };

  static_assert(FLAGS_COUNT == 15, "Flags count has been chagned. Please update the flags to match the new count.");
  
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
  Flag time_simulator = {
    .name_short = "-ts", .name_long = "--time-simulator",
    .description = "calculate the time for the simulator.",
  };
  Flag time_thoroughly = {
    .name_short = "-tith", .name_long = "--time-thoroughly",
    .description = "collect timing statistics for every part of the compiler.",
    .type = FLAG_TYPE_U64,
    .default_value.u64 = 1,
  };
  Flag tape_size = {
    .name_short = "-ts", .name_long = "--tape-size",
    .description = "specify the size of the tape (in bytes).",
    .type = FLAG_TYPE_U64,
    .default_value.u64 = TAPE_SIZE,
  };
  Flag arg_tape_size = {
    .name_short = "-ats", .name_long = "--arg-tape-size",
    .description = "specify the size of the function argument tape in bytes.",
    .type = FLAG_TYPE_U64,
    .default_value.u64 = ARG_TAPE_SIZE,
  };
  Flag ret_tape_size = {
    .name_short = "-rts", .name_long = "--ret-tape-size",
    .description = "specify the size of the return value tape in bytes.",
    .type = FLAG_TYPE_U64,
    .default_value.u64 = RET_TAPE_SIZE,
  };
  Flag global_var_tape_size = {
    .name_short = "-gvts", .name_long = "--global-var-tape-size",
    .description = "specify the size of the global variable tape in bytes.",
    .type = FLAG_TYPE_U64,
    .default_value.u64 = GLOBAL_VAR_TAPE_SIZE,
  };
  Flag local_var_tape_size = {
    .name_short = "-lvts", .name_long = "--local-var-tape-size",
    .description = "specify the size of the local variable tape in bytes.",
    .type = FLAG_TYPE_U64,
    .default_value.u64 = LOCAL_VAR_TAPE_SIZE,
  };
  Flag help = {
    .name_short = "-h", .name_long = "--help",
    .description = "present this infromation.",
  };

  static_assert(FLAGS_COUNT == 15, "Flags count has been chagned. Please update the array to match the new count.");
  Flag *flag_array[FLAGS_COUNT] = {
    [FLAG_SIMULATE] = &sim,
    [FLAG_COMPILE_FASM] = &comp,
    [FLAG_RUN] = &run,
    [FLAG_TIME_TOKENIZER] = &time_tokenization,
    [FLAG_TIME_PARSER] = &time_parsing,
    [FLAG_TIME_CODEGEN] = &time_codegen,
    [FLAG_TIME_FASM] = &time_fasm,
    [FLAG_TIME_SIMULATOR] = &time_simulator,
    [FLAG_TIME_THOROUGHLY] = &time_thoroughly,
    [FLAG_TAPE_SIZE] = &tape_size,
    [FLAG_ARG_TAPE_SIZE] = &arg_tape_size,
    [FLAG_RET_TAPE_SIZE] = &ret_tape_size,
    [FLAG_GLOBAL_VAR_TAPE_SIZE] = &global_var_tape_size,
    [FLAG_LOCAL_VAR_TAPE_SIZE] = &local_var_tape_size,
    [FLAG_HELP] = &help,
  };

  Flags flags = { .items = flag_array, .count = FLAGS_COUNT, };
  Args args = { .args = (const char**) argv, .args_count = (size_t) argc };
  if (!parse_flags(&args, &flags)) {
    fprintf(stderr, "Error: incorrect usage of `%s` flag!\n", args.arg);
    print_usage(stderr, program, "<input> ", flags);
    return 1;
  }

  if (argc < 1) {
    fprintf(stderr, "Error: input `tare` file not provided!\n");
    print_usage(stderr, program, "<input> ", flags);
    return 1;
  }
  
  if (help.value.on) {
    print_usage(stdout, program, "<input> ", flags);
    return 0;
  }

  if (run.value.on) comp.value.on = true;
  if (comp.value.on) sim.value.on = false;

  if (time_thoroughly.parsed) {
    time_tokenization.value.on = true;
    time_parsing.value.on = true;
    if (comp.value.on) time_codegen.value.on = true;
    if (run.value.on) time_fasm.value.on = true;
    if (sim.value.on) time_simulator.value.on = true;
  }

  size_t path_len = strlen(path_plain);
  if (!append_sv_to_string(&prog, src)) return 1;
  if (!append_to_string(&prog, path_plain, path_len)) return 1;
  da_append(&prog, 0);
  const char *path = path_plain;

  Paths paths = {.src = src};
  StringView build = SV_MAKE(build/);
  if (!paths_from_tare(path, &paths, build)) return 1;

  ret =
    comp_or_sim_multiple_times
    (paths, time_tokenization.value.on, time_parsing.value.on,
     time_codegen.value.on, time_fasm.value.on, time_simulator.value.on,
     run.value.on, sim.value.on, tape_size.value.u64,
     arg_tape_size.value.u64, ret_tape_size.value.u64,
     global_var_tape_size.value.u64, local_var_tape_size.value.u64,
     time_thoroughly.value.u64);

  if (prog.items) free(prog.items);
  return ret;

  return
    comp_or_sim_once
    (paths, time_tokenization.value.on, time_parsing.value.on,
     time_codegen.value.on, time_fasm.value.on, time_simulator.value.on,
     run.value.on, sim.value.on, tape_size.value.u64,
     arg_tape_size.value.u64, ret_tape_size.value.u64,
     global_var_tape_size.value.u64, local_var_tape_size.value.u64, &prog);

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
    if (time_tokenization.value.on) {
      current_action_time = ((double) end - start) / 1000000000;
      
      tokenization_timer.total += current_action_time;
      if (current_action_time > tokenization_timer.worst) tokenization_timer.worst = current_action_time;
      if (current_action_time < tokenization_timer.best) tokenization_timer.best = current_action_time;
    }
#else
    if (time_tokenization.value.on) print_elapsed_time(start, end, "Tokenization");
#endif // THOROUGH_TIMING
  
    Functions funcs = {0};
    Longs gotos = {0};
    Vars globals = {0};
    Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
    if (time_parsing.value.on) start = get_current_time();
    if (!parse_file(&p)) return 1;
    if (time_parsing.value.on) end = get_current_time();
#ifdef THOROUGH_TIMING
    if (time_parsing.value.on) {
      current_action_time = ((double) end - start) / 1000000000;
      
      parsing_timer.total += current_action_time;
      if (current_action_time > parsing_timer.worst) parsing_timer.worst = current_action_time;
      if (current_action_time < parsing_timer.best) parsing_timer.best = current_action_time;
    }
#else
    if (time_parsing.value.on) print_elapsed_time(start, end, "Parsing");
#endif // THOROUGH_TIMING

    Cmd cmd = {0};

    if (sim.value.on) {
      Tape tape = {0};
      init_tape(&tape, tape_size.value.u64);
      Tape args_sim = {0};
      init_tape(&args_sim, arg_tape_size.value.u64);
      Tape rets = {0};
      init_tape(&rets, ret_tape_size.value.u64);
      Tape globals_sim = {0};
      init_tape(&globals_sim, global_var_tape_size.value.u64);
      Tape locals = {0};
      init_tape(&locals, local_var_tape_size.value.u64);
      Longs stack = {0};
      Vids vids = {0};

      Simulator simulator = {
        .tape = &tape, .r = 8, .p = &p,
        .args = &args_sim, .rets = &rets,
        .globals = &globals_sim, .locals = &locals,
        .stack = &stack, .global_vars = p.globals,
        .vids = &vids, .fns = p.funcs,
      };

      ret = sim_tare(&simulator);

      if (tape.items) free(tape.items);
      if (args_sim.items) free(args_sim.items);
      if (rets.items) free(rets.items);
      if (globals_sim.items) free(globals_sim.items);
      if (locals.items) free(locals.items);
      if (stack.items) free(stack.items);
      if (vids.items) free(vids.items);
      if (simulator.ret_addrs.items) free(simulator.ret_addrs.items);
    } else {
      Longs longs = {0};
      Generator gen = {
        .longs = &longs, .t = &t,
        .fn = funcs.items, .op = funcs.items->items,
        .fns = &funcs,
        .globals = p.globals,
        .tape_size = tape_size.value.u64,
        .arg_tape_size = arg_tape_size.value.u64,
        .ret_tape_size = ret_tape_size.value.u64,
        .global_var_tape_size = global_var_tape_size.value.u64,
        .local_var_tape_size = local_var_tape_size.value.u64,
      };
      if (time_codegen.value.on) start = get_current_time();
      if (!gen_fasm(paths.output, &gen)) return 1;
      if (time_codegen.value.on) end = get_current_time();
      if (longs.items) free(longs.items);
      /* if (gotos.items) free(gotos.items); */
#ifdef THOROUGH_TIMING
      if (time_codegen.value.on) {
        current_action_time = ((double) end - start) / 1000000000;
      
        codegen_timer.total += current_action_time;
        if (current_action_time > codegen_timer.worst) codegen_timer.worst = current_action_time;
        if (current_action_time < codegen_timer.best) codegen_timer.best = current_action_time;
      }
#else
      if (time_codegen.value.on) print_elapsed_time(start, end, "Codegen");
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

      if (time_fasm.value.on) start = get_current_time();
      cmd_append(&cmd, "fasm", paths.fasm_input);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (time_fasm.value.on) end = get_current_time();

      if (logs) fclose(logs);
#ifdef THOROUGH_TIMING
      if (time_fasm.value.on) {
        current_action_time = ((double) end - start) / 1000000000;
      
        fasm_timer.total += current_action_time;
        if (current_action_time > fasm_timer.worst) fasm_timer.worst = current_action_time;
        if (current_action_time < fasm_timer.best) fasm_timer.best = current_action_time;
      }
#else
      if (time_fasm.value.on) print_elapsed_time(start, end, "Fasm");
#endif // THOROUGH_TIMING

      redirect.fdout = NULL;
      cmd_append(&cmd, "chmod", "+x", paths.output_bin);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (run.value.on) {
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

TARE_DEF int comp_or_sim_multiple_times(Paths paths, bool time_tokenization, bool time_parsing, bool time_codegen, bool time_fasm, bool time_simulator, bool run, bool sim, size_t tape_size, size_t arg_tape_size, size_t ret_tape_size, size_t global_var_tape_size, size_t local_var_tape_size, size_t count) {
  int ret = 0;
  ThoroughTimer tokenization_timer = { .best = 1 };
  ThoroughTimer parsing_timer      = { .best = 1 };
  ThoroughTimer codegen_timer      = { .best = 1 };
  ThoroughTimer fasm_timer         = { .best = 1 };
  ThoroughTimer sim_timer          = { .best = 1 };

  Cmd cmd = {0};

  for (unsigned int counter = 0; counter < count; counter++) {
    Tokenizer t = {0};
    size_t start = get_current_time();
    if (!tokenize_file(paths.path, &t)) return 1;
    size_t end = get_current_time();
    if (time_tokenization) time_current_action(&tokenization_timer, start, end);

    Functions funcs = {0};
    Longs gotos = {0};
    Vars globals = {0};
    Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
    if (time_parsing) start = get_current_time();
    if (!parse_file(&p)) return 1;
    if (time_parsing) end = get_current_time();
    if (time_parsing) time_current_action(&parsing_timer, start, end);

    if (sim) {
      Tape tape = {0};
      init_tape(&tape, tape_size);
      Tape args_sim = {0};
      init_tape(&args_sim, arg_tape_size);
      Tape rets = {0};
      init_tape(&rets, ret_tape_size);
      Tape globals_sim = {0};
      init_tape(&globals_sim, global_var_tape_size);
      Tape locals = {0};
      init_tape(&locals, local_var_tape_size);
      Longs stack = {0};
      Vids vids = {0};

      Simulator simulator = {
        .tape = &tape, .r = 8, .p = &p,
        .args = &args_sim, .rets = &rets,
        .globals = &globals_sim, .locals = &locals,
        .stack = &stack, .global_vars = p.globals,
        .vids = &vids, .fns = p.funcs,
      };

      if (time_simulator) start = get_current_time();
      ret = sim_tare(&simulator);
      if (time_simulator) end = get_current_time();
      if (time_simulator) time_current_action(&sim_timer, start, end);

      if (tape.items) free(tape.items);
      if (args_sim.items) free(args_sim.items);
      if (rets.items) free(rets.items);
      if (globals_sim.items) free(globals_sim.items);
      if (locals.items) free(locals.items);
      if (stack.items) free(stack.items);
      if (vids.items) free(vids.items);
      if (simulator.ret_addrs.items) free(simulator.ret_addrs.items);
    } else {
      Longs longs = {0};
      Generator gen = {
        .longs = &longs, .t = &t,
        .fn = funcs.items, .op = funcs.items->items,
        .fns = &funcs,
        .globals = p.globals,
        .tape_size = tape_size,
        .arg_tape_size = arg_tape_size,
        .ret_tape_size = ret_tape_size,
        .global_var_tape_size = global_var_tape_size,
        .local_var_tape_size = local_var_tape_size,
      };
      if (time_codegen) start = get_current_time();
      if (!gen_fasm(paths.output, &gen)) return 1;
      if (time_codegen) end = get_current_time();
      if (longs.items) free(longs.items);
      if (time_codegen) time_current_action(&codegen_timer, start, end);

      FILE *logs = fopen("logs.log", "w");
      int logs_fd = fileno(logs);
      Redirect redirect = {.fdout = &logs_fd};
      bool display = false;

      if (time_fasm) start = get_current_time();
      cmd_append(&cmd, "fasm", paths.fasm_input);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (time_fasm) end = get_current_time();

      if (logs) fclose(logs);
      if (time_fasm) time_current_action(&fasm_timer, start, end);

      redirect.fdout = NULL;
      cmd_append(&cmd, "chmod", "+x", paths.output_bin);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (run) {
        cmd_append(&cmd, paths.output_bin);
        if (!run_cmd(&cmd, redirect, display)) return 1;
      }
    }
    if (t.l.items) free(t.l.items);
    if (t.items) free(t.items);
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
  }

  if (cmd.items) free(cmd.items);
  if (paths.arena.items) free(paths.arena.items);

  double tokenization_time_average = tokenization_timer.total / count;
  double parsing_time_average = parsing_timer.total / count;
  double codegen_time_average = codegen_timer.total / count;
  double fasm_time_average = fasm_timer.total / count;
  double sim_time_average = sim_timer.total / count;

  if (time_tokenization) {
    printf("Tokenization:\n");
    printf("    Total: %lf\n", tokenization_timer.total);
    printf("    Average: %lf\n", tokenization_time_average);
    printf("    Best: %lf\n", tokenization_timer.best);
    printf("    Worst: %lf\n", tokenization_timer.worst);
  }

  if (time_parsing) {
    printf("Parsing:\n");
    printf("    Total: %lf\n", parsing_timer.total);
    printf("    Average: %lf\n", parsing_time_average);
    printf("    Best: %lf\n", parsing_timer.best);
    printf("    Worst: %lf\n", parsing_timer.worst);
  }

  if (time_codegen) {
    printf("Codegen:\n");
    printf("    Total: %lf\n", codegen_timer.total);
    printf("    Average: %lf\n", codegen_time_average);
    printf("    Best: %lf\n", codegen_timer.best);
    printf("    Worst: %lf\n", codegen_timer.worst);
  }

  if (time_fasm) {
    printf("Fasm:\n");
    printf("    Total: %lf\n", fasm_timer.total);
    printf("    Average: %lf\n", fasm_time_average);
    printf("    Best: %lf\n", fasm_timer.best);
    printf("    Worst: %lf\n", fasm_timer.worst);
  }

  if (time_simulator) {
    printf("Simulation:\n");
    printf("    Total: %lf\n", sim_timer.total);
    printf("    Average: %lf\n", sim_time_average);
    printf("    Best: %lf\n", sim_timer.best);
    printf("    Worst: %lf\n", sim_timer.worst);
  }

  return ret;
}

TARE_DEF int comp_or_sim_once(Paths paths, bool time_tokenization, bool time_parsing, bool time_codegen, bool time_fasm, bool time_simulator, bool run, bool sim, size_t tape_size, size_t arg_tape_size, size_t ret_tape_size, size_t global_var_tape_size, size_t local_var_tape_size, String *prog) {
  int ret = 0;
  Tokenizer t = {0};
  size_t start = get_current_time();
  if (!tokenize_file(paths.path, &t)) return 1;
  size_t end = get_current_time();
  if (time_tokenization) print_elapsed_time(start, end, "Tokenization");

  Functions funcs = {0};
  Longs gotos = {0};
  Vars globals = {0};
  Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
  if (time_parsing) start = get_current_time();
  if (!parse_file(&p)) return 1;
  if (time_parsing) end = get_current_time();
  if (time_parsing) print_elapsed_time(start, end, "Parsing");

  Cmd cmd = {0};

  if (sim) {
    Tape tape = {0};
    init_tape(&tape, tape_size);
    Tape args_sim = {0};
    init_tape(&args_sim, arg_tape_size);
    Tape rets = {0};
    init_tape(&rets, ret_tape_size);
    Tape globals_sim = {0};
    init_tape(&globals_sim, global_var_tape_size);
    Tape locals = {0};
    init_tape(&locals, local_var_tape_size);
    Longs stack = {0};
    Vids vids = {0};

    Simulator simulator = {
      .tape = &tape, .r = 8, .p = &p,
      .args = &args_sim, .rets = &rets,
      .globals = &globals_sim, .locals = &locals,
      .stack = &stack, .global_vars = p.globals,
      .vids = &vids, .fns = p.funcs,
    };

    if (time_simulator) start = get_current_time();
    ret = sim_tare(&simulator);
    if (time_simulator) end = get_current_time();
    if (time_simulator) print_elapsed_time(start, end, "Simulation");

    if (tape.items) free(tape.items);
    if (args_sim.items) free(args_sim.items);
    if (rets.items) free(rets.items);
    if (globals_sim.items) free(globals_sim.items);
    if (locals.items) free(locals.items);
    if (stack.items) free(stack.items);
    if (vids.items) free(vids.items);
    if (simulator.ret_addrs.items) free(simulator.ret_addrs.items);
    return ret;
  } else {
    Longs longs = {0};
    Generator gen = {
      .longs = &longs, .t = &t,
      .fn = funcs.items, .op = funcs.items->items,
      .fns = &funcs,
      .globals = p.globals,
      .tape_size = tape_size,
      .arg_tape_size = arg_tape_size,
      .ret_tape_size = ret_tape_size,
      .global_var_tape_size = global_var_tape_size,
      .local_var_tape_size = local_var_tape_size,
    };
    if (time_codegen) start = get_current_time();
    if (!gen_fasm(paths.output, &gen)) return 1;
    if (time_codegen) end = get_current_time();
    if (longs.items) free(longs.items);
    if (time_codegen) print_elapsed_time(start, end, "Codegen");

    FILE *logs = fopen("logs.log", "w");
    int logs_fd = fileno(logs);
    Redirect redirect = {.fdout = &logs_fd};
    bool display = true;

    if (time_fasm) start = get_current_time();
    cmd_append(&cmd, "fasm", paths.fasm_input);
    if (!run_cmd(&cmd, redirect, display)) return 1;
    if (time_fasm) end = get_current_time();

    if (logs) fclose(logs);
    if (time_fasm) print_elapsed_time(start, end, "Fasm");

    redirect.fdout = NULL;
    cmd_append(&cmd, "chmod", "+x", paths.output_bin);
    if (!run_cmd(&cmd, redirect, display)) return 1;
    if (run) {
      cmd_append(&cmd, paths.output_bin);
      if (!run_cmd(&cmd, redirect, display)) return 1;
    }
  }

  if (t.l.items) free(t.l.items);
  if (t.items) free(t.items);
  if (cmd.items) free(cmd.items);
  if (paths.arena.items) free(paths.arena.items);
  if (prog->items) free(prog->items);

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
  return ret;
}

TARE_DEF void time_current_action(ThoroughTimer *t, size_t start, size_t end) {
  double current_action_time = ((double) end - start) / 1000000000;
  t->total += current_action_time;
  if (current_action_time > t->worst) t->worst = current_action_time;
  if (current_action_time < t->best) t->best = current_action_time;
}
