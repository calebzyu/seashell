#include "sea.h"

int main() {
  printf("\n");

  // Main command loop
  while (1) {
    cmd_t cmd;
    memset(&cmd, 0, sizeof(cmd_t));

    int status = sea_prompt(&cmd);
    
    // Frees resources and handles non-zero returns from sea_prompt()
    sea_free(&cmd);
    if (status == CMD_EXIT) {
      exit(EXIT_SUCCESS);  // Exit program
    } else if (status) {
      sea_report_error(status);  // sea_prompt() had non-zero return value
    }
    printf("\n");
  }
  
  exit(EXIT_SUCCESS);
}