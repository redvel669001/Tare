#define COMPILER_IMPLEMENTATION
#include "compiler.h"

typedef enum {
  EXAMPLE_BASE = 0,
  EXAMPLE_TEST,
  EXAMPLE_HELLO,
  EXAMPLE_HELLO_ARABIC,
  EXAMPLE_HELLO_CHINESE,
  EXAMPLE_HELLO_HEBREW,
  EXAMPLE_HELLO_JAPANESE,
  EXAMPLE_HELLO_RUSSIAN,
  EXAMPLE_LOVE,
  EXAMPLE_IF,
  EXAMPLE_WHILE,
  EXAMPLE_HEART,
  EXAMPLE_FUNC,
  EXAMPLE_RECURSION,
  EXAMPLE_READ,
  EXAMPLE_TAPE,
  EXAMPLE_TAPE_READ,
  EXAMPLE_HEAD,
  EXAMPLES_SYSCALL,
  EXAMPLES_RETS,
  EXAMPLES_FUNC_WITH_RETS,
  EXAMPLES_PUSH,
  EXAMPLES_OPERATIONS,
  EXAMPLES_PRINT_NUM,
  EXAMPLES_FIB,
  EXAMPLES_GLOBALS,
  EXAMPLES_COUNT,
} Example;

const char *examples[EXAMPLES_COUNT] = {
  [EXAMPLE_BASE] = "./examples/base.tare",
  [EXAMPLE_TEST] = "./examples/test.tare",
  [EXAMPLE_HELLO] = "./examples/hello.tare",
  [EXAMPLE_HELLO_ARABIC] = "./examples/hello_arabic.tare",
  [EXAMPLE_HELLO_CHINESE] = "./examples/hello_chinese.tare",
  [EXAMPLE_HELLO_HEBREW] = "./examples/hello_hebrew.tare",
  [EXAMPLE_HELLO_JAPANESE] = "./examples/hello_japanese.tare",
  [EXAMPLE_HELLO_RUSSIAN] = "./examples/hello_russian.tare",
  [EXAMPLE_LOVE] = "./examples/love.tare",
  [EXAMPLE_IF] = "./examples/if.tare",
  [EXAMPLE_WHILE] = "./examples/while.tare",
  [EXAMPLE_HEART] = "./examples/heart.tare",
  [EXAMPLE_FUNC] = "./examples/func.tare",
  [EXAMPLE_RECURSION] = "./examples/recursion.tare",
  [EXAMPLE_READ] = "./examples/read.tare",
  [EXAMPLE_TAPE] = "./examples/tape.tare",
  [EXAMPLE_TAPE_READ] = "./examples/tape_read.tare",
  [EXAMPLE_HEAD] = "./examples/head.tare",
  [EXAMPLES_SYSCALL] = "./examples/syscall.tare",
  [EXAMPLES_RETS] = "./examples/rets.tare",
  [EXAMPLES_FUNC_WITH_RETS] = "./examples/func_with_rets.tare",
  [EXAMPLES_PUSH] = "./examples/push.tare",
  [EXAMPLES_OPERATIONS] = "./examples/operations.tare",
  [EXAMPLES_PRINT_NUM] = "./examples/print_num.tare",
  [EXAMPLES_FIB] = "./examples/fib.tare",
  [EXAMPLES_GLOBALS] = "./examples/globals.tare",
};

int main(int argc, char **argv) {
  const char *program = *argv;

  argc--;
  argv++;

  enum {
    FLAG_HELP = 0,
    FLAGS_COUNT,
  };

  static_assert(FLAGS_COUNT == 1, "Flags count has been chagned. Please update the flags to match the new count.");
  Flag help = {
    .name_short = "-h", .name_long = "--help",
    .description = "present this infromation.",
  };

  static_assert(FLAGS_COUNT == 1, "Flags count has been chagned. Please update the array to match the new count.");
  Flag *flag_array[FLAGS_COUNT] = {
    [FLAG_HELP] = &help,
  };

  Flags flags = { .items = flag_array, .count = FLAGS_COUNT, };
  Args args = { .args = (const char**) argv, .args_count = (size_t) argc };
  if (!parse_flags(&args, &flags)) {
    if (args.arg != NULL)
      fprintf(stderr, "Error: incorrect usage of `%s` flag!\n", args.arg);
    print_usage(stderr, program, "", flags);
    return 1;
  }

  if (help.value.on) {
    print_usage(stdout, program, "", flags);
    return 0;
  }

  Paths examples_paths[EXAMPLES_COUNT] = {0};
  StringView src = SV_MAKE(./examples/);
  StringView build = SV_MAKE(build/);

  for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
    Paths *paths = examples_paths + i;
    paths->src = src;
    if (!paths_from_tare(examples[i], paths, build)) return 1;
  }

  Cmd cmd = {0};
  Redirect redirect = {0};
  for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
    if (i == EXAMPLE_FUNC || i == EXAMPLE_READ) continue;
    Paths paths = examples_paths[i];
    printf("--------------------------------------------------\n");
    printf("%s\n", paths.path);
    cmd_append(&cmd, "./tare", paths.path);
    for (int j = 0; j < argc; j++) {
      cmd_append(&cmd, argv[j]);
    }
    if (!run_cmd(&cmd, redirect, true)) return 1;
    putchar(10);
    printf("--------------------------------------------------\n\n");
  }

  if (cmd.items) free(cmd.items);

  for(size_t i = 0; i < EXAMPLES_COUNT; i++) {
    Paths paths = examples_paths[i];
    if (paths.arena.items) free(paths.arena.items);
  }

  return 0;
}

