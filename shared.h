#ifndef SHARED_H_
#define SHARED_H_

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

#define BINGEN_IMPLEMENTATION
#include "bingen.h"

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

typedef enum {
  TARE_FLAG_SIMULATE = 0,
  TARE_FLAG_COMPILE_FASM,
  TARE_FLAG_DUMP_ASSEMBLY,
  TARE_FLAG_COMPILE_BINARY,
  TARE_FLAG_RUN,
  TARE_FLAG_TIME_TOKENIZER,
  TARE_FLAG_TIME_PARSER,
  TARE_FLAG_TIME_CODEGEN,
  TARE_FLAG_TIME_FASM,
  TARE_FLAG_TIME_BINARY,
  TARE_FLAG_TIME_SIMULATOR,
  TARE_FLAG_TIME_THOROUGHLY,
  TARE_FLAG_TAPE_SIZE,
  TARE_FLAG_ARG_TAPE_SIZE,
  TARE_FLAG_RET_TAPE_SIZE,
  TARE_FLAG_GLOBAL_VAR_TAPE_SIZE,
  TARE_FLAG_LOCAL_VAR_TAPE_SIZE,
  TARE_FLAG_HELP,
  TARE_FLAGS_COUNT,
} TareFlag;
static_assert(TARE_FLAGS_COUNT == 18, "Flags count has been chagned. Please update the flags to match the new count.");

TARE_DEF size_t find_final_dot(const char *str, size_t len);
TARE_DEF bool paths_from_tare(const char *p, Paths *ps, StringView build);

TARE_DEF size_t get_current_time(void);
TARE_DEF void print_elapsed_time(size_t start, size_t end, const char *subject);

TARE_DEF int comp_or_sim_multiple_times(Paths paths, Flags flags);
TARE_DEF int comp_or_sim_once(Paths paths, Flags flags);
TARE_DEF void time_current_action(ThoroughTimer *t, size_t start, size_t end);
TARE_DEF void print_thorough_timer(ThoroughTimer timer, size_t count, const char *name);

#endif // SHARED_H_

#ifdef SHARED_IMPLEMENTATION

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

TARE_DEF size_t get_current_time(void) {
  static struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (size_t)ts.tv_sec * 1000000000LL + (size_t)ts.tv_nsec;
}

TARE_DEF void print_elapsed_time(size_t start, size_t end, const char *subject) {
  printf("\n%s: %lf\n", subject, ((double) end - start) / 1000000000);
}

static_assert(TARE_FLAGS_COUNT == 18, "Flags count has been chagned. Please update the flags to match the new count.");
TARE_DEF int comp_or_sim_multiple_times(Paths paths, Flags flags) {
  int ret = 0;
  ThoroughTimer tokenization_timer = { .best = 1 };
  ThoroughTimer parsing_timer      = { .best = 1 };
  ThoroughTimer codegen_timer      = { .best = 1 };
  ThoroughTimer fasm_timer         = { .best = 1 };
  ThoroughTimer sim_timer          = { .best = 1 };

  Flag sim                  = *flags.items[TARE_FLAG_SIMULATE];
  Flag comp                 = *flags.items[TARE_FLAG_COMPILE_FASM];
  Flag compile_binary       = *flags.items[TARE_FLAG_COMPILE_BINARY];
  Flag run                  = *flags.items[TARE_FLAG_RUN];
  Flag time_tokenization    = *flags.items[TARE_FLAG_TIME_TOKENIZER];
  Flag time_parsing         = *flags.items[TARE_FLAG_TIME_PARSER];
  Flag time_codegen         = *flags.items[TARE_FLAG_TIME_CODEGEN];
  Flag time_fasm            = *flags.items[TARE_FLAG_TIME_FASM];
  Flag time_binary          = *flags.items[TARE_FLAG_TIME_BINARY];
  Flag time_simulator       = *flags.items[TARE_FLAG_TIME_SIMULATOR];
  Flag time_thoroughly      = *flags.items[TARE_FLAG_TIME_THOROUGHLY];
  Flag tape_size            = *flags.items[TARE_FLAG_TAPE_SIZE];
  Flag arg_tape_size        = *flags.items[TARE_FLAG_ARG_TAPE_SIZE];
  Flag ret_tape_size        = *flags.items[TARE_FLAG_RET_TAPE_SIZE];
  Flag global_var_tape_size = *flags.items[TARE_FLAG_GLOBAL_VAR_TAPE_SIZE];
  Flag local_var_tape_size  = *flags.items[TARE_FLAG_LOCAL_VAR_TAPE_SIZE];

  Cmd cmd = {0};

  size_t count = time_thoroughly.value.u64;

  for (size_t counter = 0; counter < count; counter++) {
    Tokenizer t = {0};
    size_t start = get_current_time();
    if (!tokenize_file(paths.path, &t)) return 1;
    size_t end = get_current_time();
    if (time_tokenization.value.on) time_current_action(&tokenization_timer, start, end);

    Functions funcs = {0};
    Longs gotos = {0};
    Vars globals = {0};
    Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
    if (time_parsing.value.on) start = get_current_time();
    if (!parse_file(&p)) return 1;
    if (time_parsing.value.on) end = get_current_time();
    if (time_parsing.value.on) time_current_action(&parsing_timer, start, end);

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

      if (time_simulator.value.on) start = get_current_time();
      ret = sim_tare(&simulator);
      if (time_simulator.value.on) end = get_current_time();
      if (time_simulator.value.on) time_current_action(&sim_timer, start, end);

      if (tape.items) free(tape.items);
      if (args_sim.items) free(args_sim.items);
      if (rets.items) free(rets.items);
      if (globals_sim.items) free(globals_sim.items);
      if (locals.items) free(locals.items);
      if (stack.items) free(stack.items);
      if (vids.items) free(vids.items);
      if (simulator.ret_addrs.items) free(simulator.ret_addrs.items);
    } else {
      Redirect redirect = {0};
      bool display = false;

      if (comp.value.on) {
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
        if (time_codegen.value.on) time_current_action(&codegen_timer, start, end);

        FILE *logs = fopen("logs.log", "w");
        int logs_fd = fileno(logs);
        redirect.fdout = &logs_fd;

        if (time_fasm.value.on) start = get_current_time();
        cmd_append(&cmd, "fasm", paths.fasm_input);
        if (!run_cmd(&cmd, redirect, display)) return 1;
        if (time_fasm.value.on) end = get_current_time();

        if (logs) fclose(logs);
        if (time_fasm.value.on) time_current_action(&fasm_timer, start, end);

        redirect.fdout = NULL;

      } else if (compile_binary.value.on) {
        if (time_binary.value.on) start = get_current_time();
        Longs longs = {0};
        BinaryGenerator binary_generator = {
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
        if (!gen_elf(paths.output_bin, &binary_generator)) return 1;
        if (time_binary.value.on) end = get_current_time();
      }

      if (comp.value.on || compile_binary.value.on) {
        cmd_append(&cmd, "chmod", "+x", paths.output_bin);
        if (!run_cmd(&cmd, redirect, display)) return 1;
      }
      if (run.value.on) {
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

  if (time_tokenization.value.on)
    print_thorough_timer(tokenization_timer, count, "Tokenization");

  if (time_parsing.value.on)
    print_thorough_timer(parsing_timer, count, "Parsing");

  if (time_codegen.value.on)
    print_thorough_timer(codegen_timer, count, "Codegen");

  if (time_fasm.value.on)
    print_thorough_timer(fasm_timer, count, "Fasm");

  if (time_simulator.value.on)
    print_thorough_timer(sim_timer, count, "Simulation");

  return ret;
}

static_assert(TARE_FLAGS_COUNT == 18, "Flags count has been chagned. Please update the flags to match the new count.");
TARE_DEF int comp_or_sim_once(Paths paths, Flags flags) {
  int ret = 0;

  Flag sim                  = *flags.items[TARE_FLAG_SIMULATE];
  Flag comp                 = *flags.items[TARE_FLAG_COMPILE_FASM];
  Flag compile_binary       = *flags.items[TARE_FLAG_COMPILE_BINARY];
  Flag run                  = *flags.items[TARE_FLAG_RUN];
  Flag time_tokenization    = *flags.items[TARE_FLAG_TIME_TOKENIZER];
  Flag time_parsing         = *flags.items[TARE_FLAG_TIME_PARSER];
  Flag time_codegen         = *flags.items[TARE_FLAG_TIME_CODEGEN];
  Flag time_fasm            = *flags.items[TARE_FLAG_TIME_FASM];
  Flag time_binary          = *flags.items[TARE_FLAG_TIME_BINARY];
  Flag time_simulator       = *flags.items[TARE_FLAG_TIME_SIMULATOR];
  Flag tape_size            = *flags.items[TARE_FLAG_TAPE_SIZE];
  Flag arg_tape_size        = *flags.items[TARE_FLAG_ARG_TAPE_SIZE];
  Flag ret_tape_size        = *flags.items[TARE_FLAG_RET_TAPE_SIZE];
  Flag global_var_tape_size = *flags.items[TARE_FLAG_GLOBAL_VAR_TAPE_SIZE];
  Flag local_var_tape_size  = *flags.items[TARE_FLAG_LOCAL_VAR_TAPE_SIZE];

  Tokenizer t = {0};
  size_t start = get_current_time();
  if (!tokenize_file(paths.path, &t)) return 1;
  size_t end = get_current_time();
  if (time_tokenization.value.on) print_elapsed_time(start, end, "Tokenization");

  Functions funcs = {0};
  Longs gotos = {0};
  Vars globals = {0};
  Parser p = {.t = &t, .funcs = &funcs, .gotos = &gotos, .globals = &globals};
  if (time_parsing.value.on) start = get_current_time();
  if (!parse_file(&p)) return 1;
  if (time_parsing.value.on) end = get_current_time();
  if (time_parsing.value.on) print_elapsed_time(start, end, "Parsing");

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

    if (time_simulator.value.on) start = get_current_time();
    ret = sim_tare(&simulator);
    if (time_simulator.value.on) end = get_current_time();
    if (time_simulator.value.on) print_elapsed_time(start, end, "Simulation");

    if (tape.items) free(tape.items);
    if (args_sim.items) free(args_sim.items);
    if (rets.items) free(rets.items);
    if (globals_sim.items) free(globals_sim.items);
    if (locals.items) free(locals.items);
    if (stack.items) free(stack.items);
    if (vids.items) free(vids.items);
    if (simulator.ret_addrs.items) free(simulator.ret_addrs.items);
  } else {
    Redirect redirect = {0};
    bool display = false;
    if (comp.value.on) {
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
      if (time_codegen.value.on) print_elapsed_time(start, end, "Codegen");

      FILE *logs = fopen("logs.log", "w");
      int logs_fd = fileno(logs);
      redirect.fdout = &logs_fd;

      if (time_fasm.value.on) start = get_current_time();
      cmd_append(&cmd, "fasm", paths.fasm_input);
      if (!run_cmd(&cmd, redirect, display)) return 1;
      if (time_fasm.value.on) end = get_current_time();

      if (logs) fclose(logs);
      if (time_fasm.value.on) print_elapsed_time(start, end, "Fasm");

      redirect.fdout = NULL;
    } else if (compile_binary.value.on) {
      if (time_binary.value.on) start = get_current_time();
      Longs longs = {0};
      BinaryGenerator binary_generator = {
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
      if (!gen_elf(paths.output_bin, &binary_generator)) return 1;
      if (time_binary.value.on) end = get_current_time();
    }

    if (comp.value.on || compile_binary.value.on) {
      cmd_append(&cmd, "chmod", "+x", paths.output_bin);
      if (!run_cmd(&cmd, redirect, display)) return 1;
    }
    if (run.value.on) {
      cmd_append(&cmd, paths.output_bin);
      if (!run_cmd(&cmd, redirect, display)) return 1;
    }
  }

  if (t.l.items) free(t.l.items);
  if (t.items) free(t.items);
  if (cmd.items) free(cmd.items);

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

TARE_DEF void print_thorough_timer(ThoroughTimer timer, size_t count, const char *name) {
  printf("%s:\n", name);
  printf("    Total: %lf\n", timer.total);
  printf("    Average: %lf\n", timer.total / count);
  printf("    Best: %lf\n", timer.best);
  printf("    Worst: %lf\n", timer.worst);
}

#endif // SHARED_IMPLEMENTATION

