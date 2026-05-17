#define CMD_IMPLEMENTATION
#include "cmd.h"

#define STR_IMPLEMENTATION
#include "str.h"

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

int main(void) {
  Paths examples_paths[EXAMPLES_COUNT] = {0};
  StringView src = SV_MAKE(./examples/);
  StringView build = SV_MAKE(build/);

  for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
    Paths *paths = examples_paths + i;
    paths->src = src;
    if (!paths_from_tare(examples[i], paths, build)) return 1;
  }

  bool build_examples = true;
  bool run_examples = true;

  /* printf("Testing %d tare files...\n\n", EXAMPLES_COUNT); */
  
  Cmd cmd = {0};
  Redirect redirect = {0};
  for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
    if (i == EXAMPLE_FUNC || i == EXAMPLE_READ) continue;
    Paths paths = examples_paths[i];
    
    if (build_examples) {
      printf("--------------------------------------------------\n");
      cmd_append(&cmd, "./tare", paths.path);
      if (!run_cmd(&cmd, redirect, true)) return 1;
    }

    if (run_examples) {
      cmd_append(&cmd, paths.output_bin);
      if (!run_cmd(&cmd, redirect, true)) return 1;
      putchar(10);
      printf("--------------------------------------------------\n\n");
    }
  }
  
  if (cmd.items) free(cmd.items);

  for(size_t i = 0; i < EXAMPLES_COUNT; i++) {
    Paths paths = examples_paths[i];
    if (paths.arena.items) free(paths.arena.items);
  }
  
  return 0;
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

