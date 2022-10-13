#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


void parseInput(char*[], pid_t, int*);

/* 
 * main() processes the input and executes the appropraite function
 */
int main(void) {
  
  char *args[512];
  pid_t pid = getppid(); 
  // Start loop
  while(1) {
    int argc = 0;
    parseInput(args, pid, &argc);
    // If comment or no input continue
    if (args[0][0] == "#" || args[0][0] == '\0') {
      continue;
    }

    if (!strcmp(args[0], "exit")) {
      exit(0);
    }
    
    char cwd[FILENAME_MAX];
    if (!strcmp(args[0], "cd")) {
      if (argc > 1) {
        if (chdir(args[1]) == -1) {
        } else {
          chdir(args[1]);
          }

      } else {
        chdir(getenv("HOME"));
      }
    }

    //Reset input and vars
    for (int i = 0; args[i]; i++) {
      args[i] = NULL;
    }
  }
}


/*
 * Prompts the user and parses input into an array of arguments
 */
void parseInput(char *args[], pid_t pid, int *argc) {
  
  // Max input length defined at 2048 chars
  char input[2048];
  printf(": ");
  fflush(stdout);
  fgets(input, 2048, stdin);


  // Remove trailing new line from fgets and replace with \0
  input[strcspn(input, "\n")] = '\0';
  
  // Retrun if command is nothing
  if (!strcmp(input, "")) {
    args[0] = strdup("");
    return;
  }

  // replace any instance of $$ with pid
  for (int i = 0; i < strlen(input); i++) {
    if (input[i] == '$' && input[i+1] == '$') {
      char *temp = strdup(input);
      temp[i] = '%';
      temp[i+1] = 'd';
      sprintf(input, temp, pid);
      free(temp);
    }
  }

  // Parse input into tokens and save command and arguments in args array
  char *token = strtok(input, " ");
  for (int i = 0; token; i++) {
    args[i] = strdup(token); 
    *argc += 1;

    token = strtok(NULL, " ");
  }

}
