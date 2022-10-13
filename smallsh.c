#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


void parseInput(char*[]);

/* 
 * main() processes the input and executes the appropraite function
 */
int main(void) {
  
  char *args[512];
  
  // Start loop
  while(1) {

    parseInput(args);
    
    // If comment or no input continue
    if (args[0][0] == "#" || args[0][0] == '\0') {
      continue;
    }

    if (!strcmp(args[0], "exit")) {
      exit(0);
    }

    //Reset input
    for (int i = 0; args[i]; i++) {
      args[i] = NULL;
    }
  }
}


/*
 * Prompts the user and parses input into an array of arguments
 */
void parseInput(char *args[]) {
  
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

  // Parse input into tokens and save command and arguments in args array
  char *token = strtok(input, " ");
  for (int i = 0; token; i++) {
    args[i] = strdup(token);

    token = strtok(NULL, " ");
  }
}
