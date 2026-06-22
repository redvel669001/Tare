#define SHARED_IMPLEMENTATION
#include "shared.h"

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
  [EXAMPLE_BASE] = "base.tare",
  [EXAMPLE_TEST] = "test.tare",
  [EXAMPLE_HELLO] = "hello.tare",
  [EXAMPLE_HELLO_ARABIC] = "hello_arabic.tare",
  [EXAMPLE_HELLO_CHINESE] = "hello_chinese.tare",
  [EXAMPLE_HELLO_HEBREW] = "hello_hebrew.tare",
  [EXAMPLE_HELLO_JAPANESE] = "hello_japanese.tare",
  [EXAMPLE_HELLO_RUSSIAN] = "hello_russian.tare",
  [EXAMPLE_LOVE] = "love.tare",
  [EXAMPLE_IF] = "if.tare",
  [EXAMPLE_WHILE] = "while.tare",
  [EXAMPLE_HEART] = "heart.tare",
  [EXAMPLE_FUNC] = "func.tare",
  [EXAMPLE_RECURSION] = "recursion.tare",
  [EXAMPLE_READ] = "read.tare",
  [EXAMPLE_TAPE] = "tape.tare",
  [EXAMPLE_TAPE_READ] = "tape_read.tare",
  [EXAMPLE_HEAD] = "head.tare",
  [EXAMPLES_SYSCALL] = "syscall.tare",
  [EXAMPLES_RETS] = "rets.tare",
  [EXAMPLES_FUNC_WITH_RETS] = "func_with_rets.tare",
  [EXAMPLES_PUSH] = "push.tare",
  [EXAMPLES_OPERATIONS] = "operations.tare",
  [EXAMPLES_PRINT_NUM] = "print_num.tare",
  [EXAMPLES_FIB] = "fib.tare",
  [EXAMPLES_GLOBALS] = "globals.tare",
};

int main(int argc, char **argv) {
  const char *program = *argv;

  Args args = { .args = (const char**) argv, .args_count = (size_t) argc };
  if (!parse_flags(&args, &flags)) {
    if (args.arg != NULL)
      fprintf(stderr, "Error: incorrect usage of `%s` flag!\n", args.arg);
    print_usage(stderr, program, flags);
    return 1;
  }

  if (help.value.on) {
    print_usage(stdout, program, flags);
    return 0;
  }

  Paths examples_paths[EXAMPLES_COUNT] = {0};
  StringView src = SV_MAKE(./examples/);
  StringView build = SV_MAKE(./build/);
  String arena = {0};
  da_reserve(&arena, EXAMPLES_COUNT * 64);

  for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
    Paths *paths = examples_paths + i;

    const char *example = examples[i];
    paths->input = arena.items + arena.count;
    if (!append_sv_to_string(&arena, src)) return false;
    if (!append_cstr_to_string(&arena, example)) return false;
    da_append(&arena, 0);
    paths->output = arena.items + arena.count;
    if (!append_sv_to_string(&arena, build)) return false;
    size_t final_dot = find_final_dot(example, strlen(example));
    if (!append_to_string(&arena, examples[i], final_dot)) return false;
    da_append(&arena, 0);
  }

  Cmd cmd = {0};
  Redirect redirect = {0};
  for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
    if (i == EXAMPLE_FUNC || i == EXAMPLE_READ) continue;
    Paths paths = examples_paths[i];
    assert(paths.input != NULL && "unreachable");
    assert(paths.output != NULL && "unreachable");
    printf("--------------------------------------------------\n");
    printf("%s\n", paths.input);
    cmd_append(&cmd, "./tare");
    cmd_append(&cmd, "-i", paths.input);
    cmd_append(&cmd, "-o", paths.output);
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

