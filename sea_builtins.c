#include "sea.h"

// Builtin commands
char *builtin_cmds[] = {
  "cd",
  "help",
  "exit",
  "seashell"
};

// Functions responsible for builtin commands
int (*builtin_funcs[])(cmd_t *cmd) = {
  sea_cd,
  sea_help,
  sea_exit,
  sea_seashell,
};

// Number of builtins
int num_builtins = sizeof(builtin_cmds) / sizeof(char *);


// Changes the current working directory based on the command input. If no
// directory is provided, attempts to switch to the user's home directory using
// the HOME environment variable. Returns 0 on success or CMD_CHDIR_ERR if
// changing the directory fails.
int sea_cd(cmd_t *cmd) {
  int ret;
  if (cmd->tokens[1] == NULL) {
    char *home_dir = getenv("HOME");
    if (home_dir != NULL) {
      ret = chdir(getenv("HOME"));
    } else {
      return 0;
    }
  } else {
    ret = chdir(cmd->tokens[1]);
  }
  if (ret == -1) {
    ret = CMD_CHDIR_ERR;
  }
  return ret;
}

// Help
int sea_help() {
  printf("==========  Seashell: A Unix Shell  ==========\n");
  printf("Builtin Commands:\n");
  for (int i = 0; i < num_builtins; i++) {
    printf("  %s\n", builtin_cmds[i]);
  }
  return 0;
}

// Exit shell
int sea_exit() { return CMD_EXIT; }

int sea_seashell() {
  printf("Why did the seashell refuse to code in C?\n");
  sleep(3);
  printf("It didn't want to get washed away by the wave of segfaults!\n");
  return 0;
}