#include "sea.h"

// Prompts the user for a command to execute. If at any point a function returns
// a non-zero value, sea_prompt() will return the value to main. All resource
// free'ing is done in main.
int sea_prompt(cmd_t *cmd) {
  int status;  // Tracks the return values at each step of the function

  // Print current working directory and prompt symbol
  char cwd[PATH_MAX];
  getcwd(cwd, PATH_MAX);
  char *home_dir = getenv("HOME");
  if (home_dir != NULL && strstr(cwd, home_dir)) {
    printf("~%s\n$ ", cwd + strlen(home_dir));  // Represent home directory as "~"
  } else {
    printf("%s\n$ ", cwd);  // HOME environment not set, use full current working directory
  }

  // Read input command
  status = sea_read_line(cmd);
  if (status) {
    return status;  // Error occurred or exit requested
  }

  // Split input command into tokens
  status = sea_split_line(cmd);
  if (status) {
    return status;  // Error occured
  }

  // Parse tokens
  status = sea_parse_tokens(cmd);
  // Check if both output redirection bits are set
  if (status) {
    return status;  // Error occured
  }

  // Execute command
  status = sea_execute(cmd);
  return status;
}

// Frees all malloc'd resources.
void sea_free(cmd_t *cmd) {
  if (cmd->line) {     // Check if cmd->line is malloc'd
    free(cmd->line);   // Free input line buffer
    cmd->line = NULL;  // Avoid potentially free'ing twice
  }
  if (cmd->tokens) {     // Check if cmd->tokens is malloc'd
    free(cmd->tokens);   // Free tokens array
    cmd->tokens = NULL;  // Avoid potentially free'ing twice
  }
}

// Reports custom command execution errors.
void sea_report_error(sea_status_t status) {
  switch (status) {
    case CMD_NONSTD_EXIT:
      fprintf(stderr, "sea: command had a non-standard exit\n");
      break;
    case CMD_EXEC_ERR:
      fprintf(stderr, "sea: command not found\n");  // Exec failed
      break;
    case CMD_INPREDIR_ERR:
      fprintf(stderr, "sea: failed to open file for input\n");  // Could not open file
      break;
    case CMD_OUTREDIR_ERR:
      fprintf(stderr, "sea: failed to open file for output\n");  // Could not open file
      break;
    case CMD_TRUNC_APPEND_ERR:
      fprintf(stderr, "sea: cannot truncate file (>) and append to file (>>)\n");
      break;
    case CMD_CHDIR_ERR:
      fprintf(stderr, "cd: no such file or directory\n");  // Pathname does not exist
      break;
    default:
      break;
  }
}

// Execute builtin commands or execute unix commands. Returns the return value
// of builtin function or unix command exeuction.
int sea_execute(cmd_t *cmd) {
  for (int i = 0; i < num_builtins; i++) {
    if (strcmp(cmd->name, builtin_cmds[i]) == 0) {
      return builtin_funcs[i](cmd);  // Execute built-in command and return status
    }
  }
  int ret = sea_execute_unix(cmd);
  return ret;
}

// Handles the execution of unix commands.
int sea_execute_unix(cmd_t *cmd) {
  pid_t child_pid = fork();  // Fork current process
  if (child_pid == 0) {
    // Input redirection
    if (CHECK_BIT(cmd->flags, CMD_INPREDIR)) {
      int fd_in = open(cmd->inpredir_file, O_RDONLY);  // Open file descriptor for read
      if (fd_in == -1) {
        close(fd_in);
        exit(CMD_INPREDIR_ERR);  // Failed to open file
      }
      dup2(fd_in, STDIN_FILENO);  // Redirect standard input to fd_in
    }
    // Output redirection
    if (CHECK_BIT(cmd->flags, CMD_OUTREDIR_TRUNC) || CHECK_BIT(cmd->flags, CMD_OUTREDIR_APPEND)) {
      int fd_out;
      mode_t perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
      if (CHECK_BIT(cmd->flags, CMD_OUTREDIR_TRUNC)) {
        fd_out = open(cmd->outredir_file, O_WRONLY | O_CREAT | O_TRUNC, perms);  // Truncate file
      } else {
        fd_out = open(cmd->outredir_file, O_WRONLY | O_CREAT | O_APPEND, perms);  // Append to file
      }
      if (fd_out == -1) {
        close(fd_out);
        exit(CMD_OUTREDIR_ERR);  // Failed to open file
      }
      dup2(fd_out, STDOUT_FILENO);  // Redirect standard output to fd_out
    }
    execvp(cmd->name, cmd->tokens);  // Exec command
    exit(CMD_EXEC_ERR);              // Exec failed, command does not exist
  } else {
    int status;
    waitpid(child_pid, &status, 0);         // Block until child process is done
    if (WIFEXITED(status)) {                // Check if child terminated normally
      int child_ret = WEXITSTATUS(status);  // Get child exit status
      return child_ret;                     // Return child exit status
    }
    return CMD_NONSTD_EXIT;  // Non-standard child exit (e.g., killed by a signal)
  }
}

// Parses tokens after input line is read and split into tokens.
//
// 1. If tokens[i] = "<", input redirection is desired. If tokens[i+1] is not
//    NULL, sets CMD_INPREDIR bit and input redirection filename.
//
// 2. If tokens[i] = ">", output redirection truncate is desired. If tokens[i+1]
//    is not NULL and CMD_OUTREDIR_APPEND bit is not already set, sets
//    CMD_OUTREDIR_TRUNC bit and output redirection filename. If the file
//    already exists, the contents will be deleted before writing to it.
//
// 3. If tokens[i] = ">>", output redirection append is desired. If tokens[i+1]
//    is not NULL and CMD_OUTREDIR_TRUNC bit is not already set, sets output
//    redirection bit and output redirection filename.
//
//    NOTE: Only one of ">" and ">>" should be set, not both.
//
// 4. If tokens[i] == "&", command execution in background is desired. "&" token
//    must be placed at the end of the command.
int sea_parse_tokens(cmd_t *cmd) {
  int i = 0;  // Index for cmd->tokens
  while (cmd->tokens[i] != NULL) {
    if (strcmp("<", cmd->tokens[i]) == 0 && cmd->tokens[i + 1] != NULL) {
      SET_BIT(cmd->flags, CMD_INPREDIR);               // Set input redirection bit
      remove_token(cmd->tokens, i, cmd->token_count);  // Remove "<" token
      cmd->token_count--;                              // Update token count
      cmd->inpredir_file = cmd->tokens[i];             // Set input redirection filename
      remove_token(cmd->tokens, i, cmd->token_count);  // Remove filename token
      cmd->token_count--;                              // Update token count
    } else if (strcmp(">", cmd->tokens[i]) == 0 && cmd->tokens[i + 1] != NULL) {
      if (CHECK_BIT(cmd->flags, CMD_OUTREDIR_APPEND)) {
        return CMD_TRUNC_APPEND_ERR;  // Cannot set both output redirection bits
      }
      SET_BIT(cmd->flags, CMD_OUTREDIR_TRUNC);         // Set output redirection truncate bit
      remove_token(cmd->tokens, i, cmd->token_count);  // Remove ">" token
      cmd->token_count--;                              // Update token count
      cmd->outredir_file = cmd->tokens[i];             // Set output redirection filename
      remove_token(cmd->tokens, i, cmd->token_count);  // Remove filename token
      cmd->token_count--;                              // Update token count
    } else if (strcmp(">>", cmd->tokens[i]) == 0 && cmd->tokens[i + 1] != NULL) {
      if (CHECK_BIT(cmd->flags, CMD_OUTREDIR_TRUNC)) {
        return CMD_TRUNC_APPEND_ERR;  // Cannot set both output redirection bits
      }
      SET_BIT(cmd->flags, CMD_OUTREDIR_APPEND);        // Set output redirection append bit
      remove_token(cmd->tokens, i, cmd->token_count);  // Remove ">>" token
      cmd->token_count--;                              // Update token count
      cmd->outredir_file = cmd->tokens[i];             // Set output redirection filename
      remove_token(cmd->tokens, i, cmd->token_count);  // Remove filename token
      cmd->token_count--;                              // Update token count
    } else {
      i++;
    }
  }
  return 0;
}

// Splits a command input into individual tokens, excluding delimiters. Stores
// the tokens array in the cmd_t. Returns 0 on success or -1 if
// memory allocation (malloc or realloc) fails.
int sea_split_line(cmd_t *cmd) {
  int max_token_count = INIT_TOKEN_COUNT;
  cmd->tokens = malloc(max_token_count * sizeof(char *));
  if (cmd->tokens == NULL) {
    perror("ERROR: failed to allocate memory");
    return -1;
  }
  int pos = 0;
  char *token = NULL;
  token = strtok(cmd->line, SEA_TOKEN_DELIM);
  while (token != NULL) {
    if (pos >= max_token_count - 1) {
      max_token_count *= 2;
      cmd->tokens = realloc(cmd->tokens, max_token_count * sizeof(char *));
      if (cmd->tokens == NULL) {
        perror("ERROR: failed to allocate memory");
        return -1;
      }
    }
    cmd->tokens[pos] = token;
    pos++;
    token = strtok(NULL, SEA_TOKEN_DELIM);
  }
  cmd->tokens[pos] = NULL;     // Null-terminate the tokens array
  cmd->token_count = pos + 1;  // Number of tokens
  cmd->name = cmd->tokens[0];  // First token is the command
  return 0;
}

// Reads a command from standard input. Stores the command in cmd_t->line.
// Returns 0 on success, -1 if getline() fails, or CMD_EXIT if
// EOF character is input.
int sea_read_line(cmd_t *cmd) {
  size_t line_length;
  cmd->line = NULL;
  // ret = number of characters read (including newline characters)
  // getline() allocates and reallocates the buffer, stores length of buffer in line_length
  // Buffer is null-terminated and includes newline character
  ssize_t ret = getline(&cmd->line, &line_length, stdin);
  if (ret == -1) {
    if (feof(stdin)) {  // EOF reached
      return CMD_EXIT;
    } else {
      perror("ERROR: failed to read line");
      return -1;
    }
  }
  return 0;
}

// Removes a token at index token_pos in the cmd->tokens array. Shifts the
// tokens array left by 1 starting at "pos". 
void remove_token(char **tokens, int token_pos, int token_count) {
  if (token_pos >= token_count) {  // Out of bounds position
    return; 
  }
  for (int i = token_pos; i < token_count - 1; i++) {
    tokens[i] = tokens[i + 1];
  }
}
