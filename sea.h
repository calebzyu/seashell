#ifndef SEA_H
#define SEA_H

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Macros for updating and checking flags
#define SET_BIT(flags, bitmask) (flags |= bitmask)
#define CLEAR_BIT(flags, bitmask) (flags &= ~bitmask)
#define CHECK_BIT(flags, bitmask) (flags & bitmask)

#define INIT_TOKEN_COUNT 8       // Initial token count
#define SEA_TOKEN_DELIM " \t\n"  // Token delimiters for strtok()

// cmd_t: Struct representing a shell command
typedef struct {
  char *name;           // Command name, equivalent to tokens[0]
  char *line;           // Malloc'd char array, stores a single line/command from standard input
  char **tokens;        // Dynamically allocated array of arguments (including the command)
  int token_count;      // Number of tokens, including null-terminator
  char *inpredir_file;  // Filename for input redirection, NULL if none
  char *outredir_file;  // Filename for output redirection, NULL if none
  uint32_t flags;       // Each bit represents a flag, accessed via macros
} cmd_t;

// cmd_bitmasks_t: Bitmasks for command flags
typedef enum {
  CMD_INPREDIR = (uint32_t)1 << 0,         // Input redirection
  CMD_OUTREDIR_TRUNC = (uint32_t)1 << 1,   // Output redirection truncate
  CMD_OUTREDIR_APPEND = (uint32_t)1 << 2,  // Output redirection append
} cmd_bitmasks_t;

// Shell status codes.
typedef enum {
  CMD_EXIT = 99,
  
  // Command execution errors
  CMD_NONSTD_EXIT = 101,
  CMD_EXEC_ERR = 102,
  CMD_INPREDIR_ERR = 103,
  CMD_OUTREDIR_ERR = 104,
  CMD_TRUNC_APPEND_ERR = 105,

  CMD_CHDIR_ERR = 200,
} sea_status_t;

// sea_builtins.c
extern char *builtin_cmds[];
extern int (*builtin_funcs[])(cmd_t *cmd);
extern int num_builtins;  // Number of builtin functions, set to ret value of count_builtins() in sea_builtins.c
int sea_cd(cmd_t *cmd);
int sea_help();
int sea_exit();
int sea_seashell();

// sea_funcs.c
int sea_prompt(cmd_t *cmd);
void sea_free(cmd_t *cmd);
void sea_report_error(sea_status_t status);
int sea_execute(cmd_t *cmd);
int sea_execute_unix(cmd_t *cmd);
int sea_parse_tokens(cmd_t *cmd);
int sea_split_line(cmd_t *cmd);
int sea_read_line(cmd_t *cmd);
void remove_token(char **tokens, int token_pos, int token_count);

#endif
