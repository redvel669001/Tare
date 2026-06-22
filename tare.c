#define SHARED_IMPLEMENTATION
#include "shared.h"

TARE_DEF void str_to_tare_template(const char *string); // Testing thing

int main(int argc, char **argv) {
  int ret = 0;
  const char *program = *argv;

  Args args = { .args = (const char**) argv, .args_count = (size_t) argc };
  if (!parse_flags(&args, &flags)) {
    if (args.arg != NULL)
      fprintf(stderr, "Error: incorrect usage of `%s` flag!\n", args.arg);
    print_usage(stderr, program, flags);
    return 1;
  }

  if (argc < 1) {
    fprintf(stderr, "Error: input `tare` file not provided!\n");
    print_usage(stderr, program, flags);
    return 1;
  }
  
  if (help.value.on) {
    print_usage(stdout, program, flags);
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
    if (run.value.on && !compile_binary.value.on) time_fasm.value.on = true;
    if (sim.value.on) time_simulator.value.on = true;
  }

  Paths paths = {
    .input = input.value.str,
    .output = output.value.str,
  };
  if (!paths_from_tare(&paths)) return 1;

  if (time_thoroughly.parsed) ret = comp_or_sim_multiple_times(paths);
  else ret = comp_or_sim_once(paths);
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

