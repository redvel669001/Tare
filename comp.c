// Lots of things in this header file were copied from nob.h, though
// most of these things were additionally simplified.
// https://github.com/tsoding/nob.h/blob/main/nob.h

#define CMD_IMPLEMENTATION
#include "cmd.h"

#define FLAGS_IMPLEMENTATION
#include "flags.h"

#include <sys/stat.h>

#define CFLAGS_COUNT 42
#define CFLAGS "-Wall", "-Wextra", "-pedantic", "-Wduplicated-cond", "-Wduplicated-branches", "-Wlogical-op", "-Wnull-dereference", "-Wjump-misses-init", "-Wdouble-promotion", "-Wshadow", "-Og", "-Wformat=2", "-Wformat-overflow=2", "-Wformat-truncation=2", "-Wformat-signedness", "-Winit-self", "-Wmissing-include-dirs", "-Wsync-nand", "-Wtrivial-auto-var-init", "-Wunused-const-variable=2", "-Wuse-after-free=3", "-Wstrict-flex-arrays", "-Walloc-zero", "-Wtrampolines", "-Wundef", "-Wunused-macros", "-Wbad-function-cast", "-Wcast-align", "-Wstrict-prototypes", "-Wold-style-definition", "-Wpacked", "-Wnested-externs", "-fstrict-flex-arrays", "-Wstrict-overflow=2", "-Wstringop-overflow=4", "-Warray-bounds=2", "-Warith-conversion", "-Wwrite-strings", "-Wdate-time", "-Wredundant-decls", "-Wrestrict", "-Wswitch-enum",

TARE_DEF void verify_flags(void);

TARE_DEF bool needs_rebuild(const char *program, const char *source);
TARE_DEF bool needs_rebuild_multi(const char *program, const char **sources, size_t sources_count);
TARE_DEF bool get_file_time(const char *path, struct stat *statbuf);
TARE_DEF double timespec_to_double(struct timespec tm);
TARE_DEF double get_stat_time(const struct stat *statbuf);

int main(int argc, char **argv) {
  const char *bin_path = *argv;
  const char *src_path = __FILE__;

  enum {
    FLAG_FAST = 0,
    FLAG_WERROR,
    FLAG_REBUILD,
    FLAG_NO_REBUILD,
    FLAG_REBUILD_REST,
    FLAG_DEBUG,
    FLAG_ANALYZE,
    FLAG_PROFILE,
    FLAG_TEST,
    FLAG_HELP,
    FLAGS_COUNT,
  };

  static_assert(FLAGS_COUNT == 10, "Flags count has been chagned. Please update the flags to match the new count.");
  Flag fast = {
    .name_short = "-f", .name_long = "--fast",
    .description = "use optimization when compiling [-Ofast flag].",
  };
  Flag werror = {
    .name_short = "-we", .name_long = "--werror",
    .description = "treat warnings as errors when compiling [-Werror flag].",
  };
  Flag rebuild = {
    .name_short = "-rb", .name_long = "--rebuild",
    .description = "force the compiler to rebuild itself.",
  };
  Flag no_rebuild = {
    .name_short = "-nrb", .name_long = "--no-rebuild",
    .description = "force the compiler to NOT rebuild itself.",
  };
  Flag rebuild_rest = {
    .name_short = "-rbr", .name_long = "--rebuild-rest",
    .description = "force the compiler to rebuild the project.",
  };
  Flag debug = {
    .name_short = "-g", .name_long = "--debug",
    .description = "compile with debug information [-g or --ggdb flag].",
  };
  Flag analyze = {
    .name_short = "-a", .name_long = "--analyze",
    .description = "enable static analysis during compilation [-fanalyzer]p.",
  };
  Flag profile = {
    .name_short = "-p", .name_long = "--profile",
    .description = "profile `tare` with valgrind to fix bugs and improve it.",
  };
  Flag test = {
    .name_short = "-t", .name_long = "--test",
    .description = "run some examples to verify `tare` working as intended.",
  };
  Flag help = {
    .name_short = "-h", .name_long = "--help",
    .description = "present this infromation.",
  };
  
  static_assert(FLAGS_COUNT == 10, "Flags count has been chagned. Please update the array to match the new count.");
  Flag *flag_array[FLAGS_COUNT] = {
    [FLAG_FAST] = &fast,
    [FLAG_WERROR] = &werror,
    [FLAG_REBUILD] = &rebuild,
    [FLAG_NO_REBUILD] = &no_rebuild,
    [FLAG_REBUILD_REST] = &rebuild_rest,
    [FLAG_DEBUG] = &debug,
    [FLAG_ANALYZE] = &analyze,
    [FLAG_PROFILE] = &profile,
    [FLAG_TEST] = &test,
    [FLAG_HELP] = &help,
  };

  size_t rebuild_index = FLAG_REBUILD;
  size_t no_rebuild_index = FLAG_NO_REBUILD;
  assert((strcmp(flag_array[rebuild_index]->name_short, "-rb") == 0)
         && "fix flags");
  assert((strcmp(flag_array[no_rebuild_index]->name_short, "-nrb") == 0)
         && "fix flags");
  assert((strcmp(flag_array[rebuild_index]->name_long, "--rebuild") == 0)
         && "fix flags");
  assert((strcmp(flag_array[no_rebuild_index]->name_long, "--no-rebuild") == 0)
         && "fix flags");

  Flags flags = { .items = flag_array, .count = FLAGS_COUNT, };
  Args args = { .args = (const char**) argv, .args_count = (size_t) argc };
  if (!parse_flags(&args, &flags)) {
    if (args.arg != NULL)
      fprintf(stderr, "Error: incorrect usage of `%s` flag!\n", args.arg);
    print_usage(stderr, bin_path, "", flags);
    return 1;
  }

  if (help.value.on) {
    print_usage(stdout, bin_path, "", flags);
    return 0;
  }

  if (no_rebuild.value.on) rebuild.value.on = false;

  Cmd cmd = {0};
  Redirect redirect = {0};


  enum {
    COMP_SOURCE_PATH = 0,
    COMP_SOURCE_CMD_H,
    COMP_SOURCE_FLAGS_H,
    COMP_SOURCES,
  };

  const char *comp_sources[COMP_SOURCES] = {
    [COMP_SOURCE_PATH] = src_path,
    [COMP_SOURCE_CMD_H] = "./cmd.h",
    [COMP_SOURCE_FLAGS_H] = "./flags.h",
  };
  size_t comp_sources_count = sizeof(comp_sources)/sizeof(const char*);
  assert(comp_sources_count == COMP_SOURCES);

  bool should_rebuild =
    needs_rebuild_multi(bin_path, comp_sources, comp_sources_count);
  if ((should_rebuild || rebuild.value.on) && !no_rebuild.value.on) {
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
    cmd_append(&cmd, "gcc", src_path, "-o", bin_path, CFLAGS);
    if (fast.value.on) cmd_append(&cmd, "-Ofast");
    if (werror.value.on) cmd_append(&cmd, "-Werror");
    if (debug.value.on) cmd_append(&cmd, "-ggdb");
    if (analyze.value.on) cmd_append(&cmd, "-fanalyzer");
    if (!run_cmd(&cmd, redirect, true)) return 1;
    
    if (rebuild.value.on) rebuild.value.on = false;
    no_rebuild.value.on = true;
    cmd_append(&cmd, bin_path);
    for (size_t i = 0; i < flags.count; i++) {
      Flag *flag = flags.items[i];
      if (flag->value.on) cmd_append(&cmd, flag->name_short);
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
    TARE_SOURCE_BINGEN_H,
    TARE_SOURCE_REGISTERS_H,
    TARE_SOURCE_DA_H,
    TARE_SOURCE_FLAGS_H,
    TARE_SOURCE_LEXER_H,
    TARE_SOURCE_PARSER_H,
    TARE_SOURCE_SIMULATOR_H,
    TARE_SOURCE_STR_H,
    TARE_SOURCE_TOKENIZER_H,
    TARE_SOURCE_SHARED_H,
    TARE_SOURCES,
  };
  
  enum {
    TEST_SOURCE_TEST_C = 0,
    TEST_SOURCE_CMD_H,
    TEST_SOURCE_STR_H,
    TEST_SOURCE_FLAGS_H,
    TEST_SOURCE_SHARED_H,
    TEST_SOURCES,
  };
  
  static_assert(SOURCES_COUNT == PROGRAMS_COUNT);
  const char *tare_sources[TARE_SOURCES] = {
    [TARE_SOURCE_TARE_C] = "./tare.c",
    [TARE_SOURCE_CMD_H] = "./cmd.h",
    [TARE_SOURCE_CODEGEN_H] = "./codegen.h",
    [TARE_SOURCE_BINGEN_H] = "./bingen.h",
    [TARE_SOURCE_REGISTERS_H] = "./registers.h",
    [TARE_SOURCE_DA_H] = "./da.h",
    [TARE_SOURCE_FLAGS_H] = "./flags.h",
    [TARE_SOURCE_LEXER_H] = "./lexer.h",
    [TARE_SOURCE_PARSER_H] = "./parser.h",
    [TARE_SOURCE_SIMULATOR_H] = "./simulator.h",
    [TARE_SOURCE_STR_H] = "./str.h",
    [TARE_SOURCE_TOKENIZER_H] = "./tokenizer.h",
    [TARE_SOURCE_SHARED_H] = "./shared.h",
  };
  const char *test_sources[TEST_SOURCES] = {
    [TEST_SOURCE_TEST_C] = "./test.c",
    [TEST_SOURCE_CMD_H] = "./cmd.h",
    [TEST_SOURCE_STR_H] = "./str.h",
    [TEST_SOURCE_FLAGS_H] = "./flags.h",
    [TEST_SOURCE_SHARED_H] = "./shared.h",
  };
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
    if (!rebuild_program && !rebuild_rest.value.on) {
      if (rebuild_program) printf("rebuild_program\n");
      if (rebuild.value.on) printf("rebuild\n");
      continue;
    }

    cmd_append(&cmd, "gcc", source, "-o", program, CFLAGS);
    if (fast.value.on) cmd_append(&cmd, "-Ofast");
    if (werror.value.on) cmd_append(&cmd, "-Werror");
    if (debug.value.on) cmd_append(&cmd, "-ggdb");
    if (analyze.value.on) cmd_append(&cmd, "-fanalyzer");
    if (!run_cmd(&cmd, redirect, true)) return 1;
  }

  if (test.value.on) {
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
      if (profile.value.on) cmd_append(&cmd, "valgrind", "--leak-check=full", "-s");
      cmd_append(&cmd, "./tare", examples[i]);
      if (!run_cmd(&cmd, redirect, true)) return 1;
    }
  } else {
    if (profile.value.on)
      cmd_append(&cmd, "valgrind", "--leak-check=full", "-s");
    cmd_append(&cmd, "./tare", "./examples/fib.tare", "-h");
    if (!run_cmd(&cmd, redirect, true)) return 1;
  }
  
  return 0;
}

void verify_flags(void) {
  const char *cflags[CFLAGS_COUNT] = {CFLAGS};
  for (size_t i = 0; i < CFLAGS_COUNT; i++)
    printf("cflags_extra_strict[%zu]: %s\n", i, cflags[i]);
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

