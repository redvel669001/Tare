#define SHARED_IMPLEMENTATION
#include "shared.h"

TARE_DEF void str_to_tare_template(const char *string); // Testing thing

int main(int argc, char **argv) {
  int ret = 0;

  StringView src = SV_MAKE(./examples/);

  const char *program = *argv;

  argc--;
  argv++;
  
  const char *path_plain = *argv;

  Flag sim = {
    .name_short = "-s", .name_long = "--simulate",
    .description = "invoke an interpreter instead of a compiler.",
  };
  Flag comp = {
    .name_short = "-c", .name_long = "--compile",
    .description = "compile a native executable.",
    .default_value.on = true,
  };
  Flag dump_asm = {
    .name_short = "-da", .name_long = "--dump-assembly",
    .description = "translate to assembly (fasm) but do not assemble.",
  };
  Flag compile_binary = {
    .name_short = "-b", .name_long = "--binary",
    .description = "translate directly to binary rather than assembly (fasm).",
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
  Flag time_binary = {
    .name_short = "-tb", .name_long = "--time-binary",
    .description = "calculate the time for the binary translation (not fasm).",
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

  static_assert(TARE_FLAGS_COUNT == 18, "Flags count has been chagned. Please update the array to match the new count.");
  Flag *flag_array[TARE_FLAGS_COUNT] = {
    [TARE_FLAG_SIMULATE] = &sim,
    [TARE_FLAG_COMPILE_FASM] = &comp,
    [TARE_FLAG_DUMP_ASSEMBLY] = &dump_asm,
    [TARE_FLAG_COMPILE_BINARY] = &compile_binary,
    [TARE_FLAG_RUN] = &run,
    [TARE_FLAG_TIME_TOKENIZER] = &time_tokenization,
    [TARE_FLAG_TIME_PARSER] = &time_parsing,
    [TARE_FLAG_TIME_CODEGEN] = &time_codegen,
    [TARE_FLAG_TIME_FASM] = &time_fasm,
    [TARE_FLAG_TIME_BINARY] = &time_binary,
    [TARE_FLAG_TIME_SIMULATOR] = &time_simulator,
    [TARE_FLAG_TIME_THOROUGHLY] = &time_thoroughly,
    [TARE_FLAG_TAPE_SIZE] = &tape_size,
    [TARE_FLAG_ARG_TAPE_SIZE] = &arg_tape_size,
    [TARE_FLAG_RET_TAPE_SIZE] = &ret_tape_size,
    [TARE_FLAG_GLOBAL_VAR_TAPE_SIZE] = &global_var_tape_size,
    [TARE_FLAG_LOCAL_VAR_TAPE_SIZE] = &local_var_tape_size,
    [TARE_FLAG_HELP] = &help,
  };

  Flags flags = { .items = flag_array, .count = TARE_FLAGS_COUNT, };
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

  if (sim.value.on || dump_asm.value.on
      || compile_binary.value.on) comp.value.on = false;
  if (dump_asm.value.on || compile_binary.value.on) sim.value.on = false;
  if (compile_binary.value.on) dump_asm.value.on = false;
  if (!comp.value.on && !compile_binary.value.on) run.value.on = false;

  if (!comp.value.on && !dump_asm.value.on
      && !compile_binary.value.on) time_codegen.value.on = false;
  if (!compile_binary.value.on) time_binary.value.on = false;
  if (!comp.value.on) time_fasm.value.on = false;
  if (!sim.value.on) time_simulator.value.on = false;

  if (time_thoroughly.parsed) {
    time_tokenization.value.on = true;
    time_parsing.value.on = true;
    if ((comp.value.on || dump_asm.value.on)
        && !compile_binary.value.on) time_codegen.value.on = true;
    if (compile_binary.value.on) time_binary.value.on = true;
    if (run.value.on) time_fasm.value.on = true;
    if (sim.value.on) time_simulator.value.on = true;
  }

  const char *path = path_plain;

  Paths paths = {.src = src};
  StringView build = SV_MAKE(build/);
  if (!paths_from_tare(path, &paths, build)) return 1;

  if (time_thoroughly.parsed) ret = comp_or_sim_multiple_times(paths, flags);
  else ret = comp_or_sim_once(paths, flags);
  if (paths.arena.items) free(paths.arena.items);
  return ret;
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

