#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "smallsh.h"


/* 
 * main() processes the input and executes commands in main loop
 */
int main(void) {
  
  
  // Initalize NULL to make execv call easier
  char *args[MAX_ARGS] = {NULL};
  pid_t pid = getpid();
  int exitStatus = 0;
  
   // parent must ignore control-C (SIGINT)
  struct sigaction SIGINT_action = {0};
  SIGINT_action.sa_handler = SIG_IGN;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;
  sigaction(SIGINT, &SIGINT_action, NULL);
  
  // Set up control-z signal handling
  struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = handleSIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);


  // Start main shell loop
  while(1) {
       
    // These vars are reset each loop
    int argc = 0;
    int isBackground = 0;
    pid_t childPid;
    char *inFile = NULL;
    char *outFile = NULL;
    
      
    // Print out any completed background processes before prompting for additional commands
    while ((childPid = waitpid(-1, &exitStatus, WNOHANG)) > 0) { 
      if (WIFEXITED(exitStatus)) {
          printf("Background process %d terminated with status %d\n", childPid, WEXITSTATUS(exitStatus));
          fflush(stdout);
      } 
      if (WIFSIGNALED(exitStatus)) {
          printf("Background process %d terminated with signal %d\n", childPid, WTERMSIG(exitStatus));
          fflush(stdout);
      } 

    }
  
    // Parse the input
    parseInput(args, pid, &argc, &isBackground, &inFile, &outFile);

    // If comment or no input, continue
    if (args[0][0] == '#' || argc == 0) {
      // Reset the args array to null pointers after freeing mem
      // Requred for comments, for example # ls -a will save ls -a in args, so must be cleared
      // TODO: Probably should handle this in the parseInput function
      for (int i = 0; i < MAX_ARGS; i++) {
      free(args[i]);
      args[i] = NULL;
      }
      continue;
    }

    // Process exit command
    else if (!strcmp(args[0], "exit")) {
      //TODO: free any child processes - require keeping track in array?

      // Free memory before exit
      free(inFile);
      free(outFile);
      for (int i = 0; i < MAX_ARGS; i++) {
        free(args[i]);
      }
      exit(0);
    }
   
    // Process cd command
    else if (!strcmp(args[0], "cd")) {
      if (argc > 1) {
        if (chdir(args[1]) == -1) {
          printf("Directory not found.\n");
          fflush(stdout);
        } else {
          chdir(args[1]);
          }

      } else {
        chdir(getenv("HOME"));
      }
    }
    
    // Process status command
    else if (!strcmp(args[0], "status")) {
      if (WIFEXITED(exitStatus)) {
        printf("exit status %d\n", WEXITSTATUS(exitStatus));
        fflush(stdout);
      } 
      if (WIFSIGNALED(exitStatus)) {
        printf("terminated with signal %d\n", WTERMSIG(exitStatus));
        fflush(stdout);
      }  
    }
    
    // All other commands
    else {
      runCommand(args, &exitStatus, &isBackground, inFile, outFile);
    }

    // Before next prompt: free memory for file names if there was one
    if (inFile) {
      free(inFile);
    }
    if (outFile) {
      free(outFile);
    }
    // Reset the args array to null pointers after freeing mem
    for (int i = 0; i < MAX_ARGS; i++) {
      free(args[i]);
      args[i] = NULL;
    }

  }
  return 0;
}
