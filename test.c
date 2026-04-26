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
  EXAMPLE_TEST = 0,
  EXAMPLE_HELLO,
  EXAMPLE_HELLO_ARABIC,
  EXAMPLE_HELLO_CHINESE,
  EXAMPLE_HELLO_HEBREW,
  Example_HELLO_JAPANESE,
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
  EXAMPLES_COUNT,
} Example;

const char *examples[EXAMPLES_COUNT] = {
  "./examples/test.tare",
  "./examples/hello.tare",
  "./examples/hello_arabic.tare",
  "./examples/hello_chinese.tare",
  "./examples/hello_hebrew.tare",
  "./examples/hello_japanese.tare",
  "./examples/hello_russian.tare",
  "./examples/love.tare",
  "./examples/if.tare",
  "./examples/while.tare",
  "./examples/heart.tare",
  "./examples/func.tare",
  "./examples/recursion.tare",
  "./examples/read.tare",
  "./examples/tape.tare",
  "./examples/tape_read.tare",
  "./examples/head.tare",
  "./examples/syscall.tare",
  "./examples/rets.tare",
  "./examples/func_with_rets.tare",
  "./examples/push.tare",
  "./examples/operations.tare",
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

