// Lots of things in this header file were copied from nob.h, though
// most of these things were additionally simplified.
// https://github.com/tsoding/nob.h/blob/main/nob.h

#define CMD_IMPLEMENTATION
#include "cmd.h"

#include <sys/stat.h>

#define CFLAGS_BASIC "-Wextra", "-Wall", "-pedantic"

#define CFLAGS_STRICT "-Wall", "-Wextra", "-pedantic", "-Wduplicated-cond", "-Wduplicated-branches", "-Wlogical-op", "-Wnull-dereference", "-Wjump-misses-init", "-Wdouble-promotion", "-Wshadow"

#define CFLAGS_EXTRA_STRICT "-Wall", "-Wextra", "-pedantic", "-Wduplicated-cond", "-Wduplicated-branches", "-Wlogical-op", "-Wnull-dereference", "-Wjump-misses-init", "-Wdouble-promotion", "-Wshadow", "-Og", "-Wformat=2", "-Wformat-overflow=2", "-Wformat-truncation=2", "-Wformat-signedness", "-Winit-self", "-Wmissing-include-dirs", "-Wsync-nand", "-Wtrivial-auto-var-init", "-Wunused-const-variable=2", "-Wuse-after-free=3", "-Wstrict-flex-arrays", "-Walloc-zero", "-Wtrampolines", "-Wundef", "-Wunused-macros", "-Wbad-function-cast", "-Wcast-align", "-Wstrict-prototypes", "-Wold-style-definition", "-Wpacked", "-Wnested-externs", "-fstrict-flex-arrays", "-Wstrict-overflow=2", "-Wstringop-overflow=4", "-Warray-bounds=2", "-Warith-conversion", "-Wwrite-strings", "-Wdate-time", "-Wredundant-decls", "-Wrestrict", "-Wswitch-enum"

TARE_DEF void verify_flags(void);

TARE_DEF bool needs_rebuild(const char *program, const char *source);
TARE_DEF bool needs_rebuild_multi(const char *program, const char **sources, size_t sources_count);
TARE_DEF bool get_file_time(const char *path, struct stat *statbuf);
TARE_DEF double timespec_to_double(struct timespec tm);
TARE_DEF double get_stat_time(const struct stat *statbuf);

int main(int argc, char **argv) {
  bool basic = true;
  bool strict = false;
  bool extra_strict = false;
  bool fast = false;
  bool werror = false;
  bool no_werror = false;
  bool rebuild = false;
  bool no_rebuild = false;
  bool rebuild_rest = false;
  bool no_rebuild_rest = false;
  bool debug = false;
  bool no_debug = false;
  bool profile = false;
  bool no_profile = false;
  bool test = false;
  bool no_test = false;

  enum {
    FLAG_BASIC = 0,
    FLAG_STRICT,
    FLAG_EXTRA_STRICT,
    FLAG_FAST,
    FLAG_WERROR,
    FLAG_NO_WERROR,
    FLAG_REBUILD,
    FLAG_NO_REBUILD,
    FLAG_REBUILD_REST,
    FLAG_NO_REBUILD_REST,
    FLAG_DEBUG,
    FLAG_NO_DEBUG,
    FLAG_PROFILE,
    FLAG_NO_PROFILE,
    FLAG_TEST,
    FLAG_NO_TEST,
    
    FLAGS_COUNT,
  };
  
  const char *flags_short_names[FLAGS_COUNT] = {
    "-b", "-s", "-es", "-f",
    "-we", "-nwe",
    "-rb", "-nrb",
    "-rbr", "-nrbr",
    "-g", "-ng",
    "-p", "-np",
    "-t", "-nt",
  };
  
  const char *flags_long_names[FLAGS_COUNT] = {
    "--basic", "--strict", "--extra-strict", "--fast",
    "--werror", "--no-werror",
    "--rebuild", "--no-rebuild",
    "--rebuild-rest", "--no-rebuild-rest",
    "--debug", "--no-debug",
    "--profile", "--no-profile",
    "--test", "--no-test",
  };

  
  bool *flags[FLAGS_COUNT] = {
    &basic,
    &strict,
    &extra_strict,
    &fast,
    &werror, &no_werror,
    &rebuild, &no_rebuild,
    &rebuild_rest, &no_rebuild_rest,
    &debug, &no_debug,
    &profile, &no_profile,
    &test, &no_test,
  };

  size_t rebuild_index = FLAG_REBUILD;
  size_t no_rebuild_index = FLAG_NO_REBUILD;
  assert((strcmp(flags_short_names[rebuild_index], "-rb") == 0)
         && "fix flags");
  assert((strcmp(flags_short_names[no_rebuild_index], "-nrb") == 0)
         && "fix flags");
  assert((strcmp(flags_long_names[rebuild_index], "--rebuild") == 0)
         && "fix flags");
  assert((strcmp(flags_long_names[no_rebuild_index], "--no-rebuild") == 0)
         && "fix flags");
  const char *rebuild_flag_short = flags_short_names[rebuild_index];
  const char *no_rebuild_flag_short = flags_short_names[no_rebuild_index];
  const char *rebuild_flag_long = flags_long_names[rebuild_index];
  const char *no_rebuild_flag_long = flags_long_names[no_rebuild_index];

  // start from 1 to skip program name
  for (size_t i = 1; i < (size_t) argc; i++) {
    const char *flag = argv[i];
    for (size_t j = 0; j < FLAGS_COUNT; j++) {
      if ((strcmp(flag, flags_short_names[j]) == 0) ||
          (strcmp(flag, flags_long_names[j]) == 0)) *flags[j] = true;
    }
  }

  if (extra_strict) {
    strict = false;
    basic = false;
  }
  
  if (strict) basic = false;

  if (no_werror) werror = false;
  if (no_debug) debug = false;
  if (no_rebuild) rebuild = false;
  if (no_rebuild_rest) rebuild_rest = false;
  if (no_profile) profile = false;

  Cmd cmd = {0};
  Redirect redirect = {0};

  const char *bin_path = *argv;
  const char *src_path = __FILE__;
  if ((needs_rebuild(bin_path, src_path) || rebuild) && !no_rebuild) {
    // Is this really necessary?
    // Rename old binary for backup
    /* size_t old_bin_path_len = snprintf(NULL, 0, "%s.old", bin_path); */
    /* char *old_bin_path = malloc(old_bin_path_len + 1); */
    /* if (sprintf(old_bin_path, "%s.old", bin_path) < 0) return 1; */
    /* printf("Renaming %s to %s for backup...\n", bin_path, old_bin_path); */
    /* if (rename(bin_path, old_bin_path) < 0) { */
    /*   fprintf(stderr, "ERROR: could not rename %s to %s: %s", */
    /*           bin_path, old_bin_path, strerror(errno)); */
    /*   return 1; */
    /* } */

    printf("Recompiling...\n");
    cmd_append(&cmd, "gcc", src_path, "-o", bin_path);
    if (basic) cmd_append(&cmd, CFLAGS_BASIC);
    else if (strict) cmd_append(&cmd, CFLAGS_STRICT);
    else if (extra_strict) cmd_append(&cmd, CFLAGS_EXTRA_STRICT);
    if (fast) cmd_append(&cmd, "-Ofast");
    if (werror) cmd_append(&cmd, "-Werror");
    if (debug) cmd_append(&cmd, "-ggdb");
    if (!run_cmd(&cmd, redirect, true)) return 1;
    
    for (size_t i = 0; i < (size_t) argc; i++) {
      const char *flag = argv[i];
      if (strcmp(flag, rebuild_flag_short) == 0) {
        flag = no_rebuild_flag_short;
      } else if (strcmp(flag, rebuild_flag_long) == 0) {
        flag = no_rebuild_flag_long;
      }
      cmd_append(&cmd, flag);
    }
    if (!run_cmd(&cmd, redirect, true)) return 1;
    return 0;
  }
  
#define SOURCES_COUNT 2
#define PROGRAMS_COUNT 2

  enum {
    TARE_SOURCE_TARE_C = 0,
    TARE_SOURCE_CMD_H,
    TARE_SOURCE_CODEGEN_H,
    TARE_SOURCE_DA_H,
    TARE_SOURCE_FLAGS_H,
    TARE_SOURCE_LEXER_H,
    TARE_SOURCE_PARSER_H,
    TARE_SOURCE_SIMULATOR_H,
    TARE_SOURCE_STR_H,
    TARE_SOURCE_TOKENIZER_H,
    TARE_SOURCES,
  };
  
  enum {
    TEST_SOURCE_TEST_C = 0,
    TEST_SOURCE_CMD_H,
    TEST_SOURCE_STR_H,
    TEST_SOURCES,
  };
  
  static_assert(SOURCES_COUNT == PROGRAMS_COUNT);
  const char *tare_sources[] = {"./tare.c", "./cmd.h", "codegen.h", "./da.h", "./flags.h", "./lexer.h", "./parser.h", "./simulator.h", "./str.h", "./tokenizer.h"};
  const char *test_sources[] = {"./test.c", "./cmd.h", "./str.h"};
  size_t tare_sources_count = sizeof(tare_sources)/sizeof(const char*);
  size_t test_sources_count = sizeof(test_sources)/sizeof(const char*);
  assert(tare_sources_count == TARE_SOURCES);
  assert(test_sources_count == TEST_SOURCES);
  size_t sources_counts[2] = {tare_sources_count, test_sources_count};
  
  const char **sources_multi[SOURCES_COUNT] = {tare_sources, test_sources};
  const char *programs[PROGRAMS_COUNT] = {"tare", "test"};

  for (size_t i = 0; i < SOURCES_COUNT; i++) {
    const char *program = programs[i];
    
    const char **sources = sources_multi[i];
    const char *source = sources[0];
    size_t sources_count = sources_counts[i];
    bool rebuild_program = needs_rebuild_multi(program, sources, sources_count);
    if (!rebuild_program && !rebuild_rest) {
      if (rebuild_program) printf("rebuild_program\n");
      if (rebuild) printf("rebuild\n");
      continue;
    }

    cmd_append(&cmd, "gcc", source, "-o", program);
    if (basic) cmd_append(&cmd, CFLAGS_BASIC);
    else if (strict) cmd_append(&cmd, CFLAGS_STRICT);
    else if (extra_strict) cmd_append(&cmd, CFLAGS_EXTRA_STRICT);

    if (fast) cmd_append(&cmd, "-Ofast");
    if (werror) cmd_append(&cmd, "-Werror");
    if (debug) cmd_append(&cmd, "-ggdb");
    if (!run_cmd(&cmd, redirect, true)) return 1;
  }

  if (test) {
    enum {
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
      EXAMPLE_SYSCALL,
      EXAMPLE_RETS,
      EXAMPLE_FUNC_WITH_RETS,
      EXAMPLE_PUSH,
      EXAMPLE_OPERATIONS,
      EXAMPLE_PRINT_NUM,
      EXAMPLE_FIB,
      EXAMPLE_GLOBALS,
      EXAMPLES_COUNT,
    };

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
      [EXAMPLE_SYSCALL] = "./examples/syscall.tare",
      [EXAMPLE_RETS] = "./examples/rets.tare",
      [EXAMPLE_FUNC_WITH_RETS] = "./examples/func_with_rets.tare",
      [EXAMPLE_PUSH] = "./examples/push.tare",
      [EXAMPLE_OPERATIONS] = "./examples/operations.tare",
      [EXAMPLE_PRINT_NUM] = "./examples/print_num.tare",
      [EXAMPLE_FIB] = "./examples/fib.tare",
      [EXAMPLE_GLOBALS] = "./examples/globals.tare",
    };

    for (size_t i = 0; i < EXAMPLES_COUNT; i++) {
      if (profile) cmd_append(&cmd, "valgrind", "--leak-check=full");
      cmd_append(&cmd, "./tare", examples[i]);
      if (!run_cmd(&cmd, redirect, true)) return 1;
    }
  } else {
    // Temporary, for testing the WIP parser.
    if (profile) cmd_append(&cmd, "valgrind", "--leak-check=full");
    cmd_append(&cmd, "./tare", "./examples/recursion.tare");
    if (!run_cmd(&cmd, redirect, true)) return 1;
  }
  
  return 0;
}

void verify_flags(void) {
#define CFB 3
#define CFS 10
#define CFES 42

  const char *cflags_basic[CFB] = {CFLAGS_BASIC};
  const char *cflags_strict[CFS] = {CFLAGS_STRICT};
  const char *cflags_extra_strict[CFES] = {CFLAGS_EXTRA_STRICT};
 
  // cflags_basic
  for (size_t i = 0; i < CFB; i++)
    printf("cflags_basic[%zu]: %s\n", i, cflags_basic[i]);
  
  // cflags_strict
  for (size_t i = 0; i < CFS; i++)
    printf("cflags_strict[%zu]: %s\n", i, cflags_strict[i]);

  // cflags_extra_strict
  for (size_t i = 0; i < CFS; i++)
    printf("cflags_extra_strict[%zu]: %s\n", i, cflags_extra_strict[i]);
}

TARE_DEF bool needs_rebuild(const char *program, const char *source) {
  return needs_rebuild_multi(program, &source, 1);
}

TARE_DEF bool needs_rebuild_multi(const char *program, const char **sources, size_t sources_count) {
  double source_modified = 0;
  struct stat source_statbuf = {0};
  
  for (size_t i = 0; i < sources_count; i++) {
    if (!get_file_time(sources[i], &source_statbuf)) exit(1);
    double last_modified = timespec_to_double(source_statbuf.st_mtim);
    if (last_modified > source_modified) source_modified = last_modified;
  }

  struct stat program_statbuf = {0};
  if (!get_file_time(program, &program_statbuf)) exit(1);
  if (errno == ENOENT) return true; // Refer to the error check in get_file_time
  double program_modified = timespec_to_double(program_statbuf.st_mtim);
  return source_modified > program_modified;
}

TARE_DEF bool get_file_time(const char *path, struct stat *statbuf) {
  int result = stat(path, statbuf);
  if (result == -1) {
    fprintf(stderr, "ERROR: couldn't get time for %s: %s\n", path, strerror(errno));
    // If the file cannot be found, this could mean the binary
    // executable doesn't exist when its last modified time is grabbed
    // for comparison with any of the source code files, so this
    // should return true since a missing binary file output should
    // mean recompilation is, in fact, necessary.
    if (errno == ENOENT) return true;
    return false;
  }
  return true;
}

TARE_DEF double timespec_to_double(struct timespec tm) {
  return (double) tm.tv_sec + (double)(tm.tv_nsec)/1000000000;
}

TARE_DEF double get_stat_time(const struct stat *statbuf) {
  return timespec_to_double(statbuf->st_mtim);
}

