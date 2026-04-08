#ifndef CMD_H_
#define CMD_H_

// Lots of things in this header file were copied from nob.h, though
// most of these things were additionally simplified.
// https://github.com/tsoding/nob.h/blob/main/nob.h

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "da.h"

#define cmd_append(cmd, ...)                    \
  da_append_many(cmd,                                  \
                 ((const char*[]){__VA_ARGS__}),                        \
                 (sizeof((const char*[]){__VA_ARGS__})/sizeof(const char*)))

typedef struct {
  const char **items;
  size_t count;
  size_t capacity;
} Cmd;

typedef struct {
  int *fdin;
  int *fdout;
  int *fderr;
} Redirect;

TARE_DEF void display_cmd(Cmd *cmd);
TARE_DEF bool run_cmd(Cmd *cmd, Redirect redirect, bool display);
TARE_DEF bool run_cmd_(Cmd *cmd, Redirect redirect, bool display);

#endif // CMD_H_

#ifdef CMD_IMPLEMENTATION

TARE_DEF void display_cmd(Cmd *cmd) {
  printf("[CMD] ");
  for (size_t i = 0; i < cmd->count; i++) {
    const char *command = cmd->items[i];
    if (command == NULL) continue;
    printf("%s ", command);
  }
  putchar(10);
}

TARE_DEF bool run_cmd(Cmd *cmd, Redirect redirect, bool display) {
  assert(cmd != NULL);
  assert(cmd->items != NULL);
  da_append(cmd, NULL);
  bool success = run_cmd_(cmd, redirect, display);
  cmd->count = 0;
  return success;
}

TARE_DEF bool run_cmd_(Cmd *cmd, Redirect redirect, bool display) {
  if (display) display_cmd(cmd);
  pid_t cpid = fork();
  if (cpid < 0) {
    fprintf(stderr,
            "[ERROR]: Could not fork child process: %s\n",
            strerror(errno));
    return false;
  }

  if (cpid == 0) {
    if (redirect.fdin) {
      if (dup2(*redirect.fdin, STDIN_FILENO) < 0) {
        fprintf(stderr, "[ERROR] Could not setup stdin for child process: %s\n", strerror(errno));
        exit(1);
      }
    }

    if (redirect.fdout) {
      if (dup2(*redirect.fdout, STDOUT_FILENO) < 0) {
        fprintf(stderr, "[ERROR] Could not setup stdout for child process: %s\n", strerror(errno));
        exit(1);
      }
    }

    if (redirect.fderr) {
      if (dup2(*redirect.fderr, STDERR_FILENO) < 0) {
        fprintf(stderr, "[ERROR] Could not setup stderr for child process: %s", strerror(errno));
        exit(1);
      }
    }

    if (execvp(cmd->items[0], (char *const *)cmd->items) < 0) {
      fprintf(stderr, "ERROR: Could not exec child process for %s: %s\n", cmd->items[0], strerror(errno));
      return false;
    }
  }

  for (;;) {
    int wstatus = 0;
    if (waitpid(cpid, &wstatus, 0) < 0) {
      fprintf(stderr, "could not wait on command (pid %d): %s", cpid, strerror(errno));
      return false;
    }

    if (WIFEXITED(wstatus)) {
      int exit_status = WEXITSTATUS(wstatus);
      if (exit_status != 0) {
        fprintf(stderr, "command exited with exit code %d", exit_status);
        return false;
      }

      break;
    }

    if (WIFSIGNALED(wstatus)) {
      fprintf(stderr, "command process was terminated by signal %d", WTERMSIG(wstatus));
      return false;
    }
  }

  if (redirect.fdin) {
    close(*redirect.fdin);
    *redirect.fdin = -1;
  }
  if (redirect.fdout) {
    close(*redirect.fdout);
    *redirect.fdout = -1;
  }
  if (redirect.fderr) {
    close(*redirect.fderr);
    *redirect.fderr = -1;
  }
  
  return true;
}

#endif // CMD_IMPLEMENTATION

