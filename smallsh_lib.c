#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "smallsh.h"

// Global variable to toggle SIGTSTP - I don't think there is another way to handle this;
// type from The Linux Programming Interface 21.1.3
volatile sig_atomic_t allowBG = 1;

/*
 * Prompts the user and parses input into an array of arguments
 *
 * params:
 *  args:          array of arguments
 *  pid:           shell pid
 *  argc:          number of arguments passed
 *  isBackground:  checks if command should be a background process or not
 *  inFile:        pointer to string to store the name of inFile if there is an input redirect
 *  outFile:       pointer to string to store the name of outFile if there is an output redirect
 */

void parseInput(char *args[], pid_t pid, int *argc, int *isBackground, char **inFile, char **outFile) {
  

  // Max input length defined at 2048 chars
  char input[MAX_IN] = {'\0'};

  printf(": ");
  fflush(stdout);
  fgets(input, MAX_IN, stdin);

  // Remove trailing new line from fgets and replace with \0
  input[strcspn(input, "\n")] = '\0';
  
  // Return if command is nothing
  if (!strcmp(input, "")) {
    args[0] = malloc(strlen("")+1);
    strcpy(args[0], "");
    return;
  }

  // replace any instance of $$ with pid by using string formatting
  // Referenced from https://en.cppreference.com/w/c/io/fprintf
  for (unsigned int i = 0; i < strlen(input); i++) {
    if (input[i] == '$' && input[i+1] == '$') {
      char *temp = malloc(strlen(input)+1);
      strcpy(temp, input);
      temp[i] = '%';
      temp[i+1] = 'd';      
      snprintf(input, MAX_IN, temp, pid);
      free(temp);
    }
  }

  // Parse input into tokens and save command and arguments in args array
  char *token = strtok(input, " ");
  char *lastWord = malloc(strlen("&")+1);
  for (int i = 0; token; i++) {
    lastWord = realloc(lastWord, (strlen(token)+1));
    strcpy(lastWord, token);
    // Check for output redirect
    if (strcmp(token, ">") == 0) {
      // Advance the token early to get file name
      token = strtok(NULL, " ");
      // Allocate memory for file name string, will be freed in caller
      *outFile = malloc((strlen(token)+1));
      strcpy(*outFile, token);
      i -= 1; // Decrement for case when more commands, i.e. echo > junk foobar
    }
    // Check for input redirect 
    else if (strcmp(token, "<") == 0) {
      token = strtok(NULL, " ");
      *inFile = malloc((strlen(token)+1));
      strcpy(*inFile, token);
      i -= 1;
    }
    // add to args
    else {
      args[i] = malloc((strlen(token)+1));
      *argc += 1;
      strcpy(args[i], token);
    }
    token = strtok(NULL, " ");
  }
  
  // set process as background process if & and allowBG is enabled
  if (!strcmp(lastWord, "&")) {
    if (allowBG) {
      *isBackground = 1;
    }
    
    // Remove flag so command will exec properly
    args[*argc-1] = NULL;
  }
  
  // Free the last word temp var
  free(lastWord);
  
  // reset input (did not think this was necessary until 
  // processing signals and control-z caused use of last input)
  memset(input, '\0', MAX_IN);
}

/*
 * Runs commands other than exit, cd, and status
 *
 * params:
 *  args:         array of arguments
 *  exitStatus:   exitStatus of process 
 *  isBackground: check if command should be executed as background process
 *  inFile:       name of inFile if there is an input redirect
 *  outFile:      name of outFile if there is an output redirect
 */

void runCommand(char *args[], int *exitStatus, int *isBackground, char *inFile, char *outFile) {
  
  /* 
   *  Set up blocking of SIGTSTP so this signal is ignored by child foreground and background processes
   *  Blocked signals are delivered after being unblocked, so SIGTSTP will be delivered after 
   *  the process is complete per the assignment specifications
   *  Reference: Linux Programming Interface book 20.11  
  */

  sigset_t sigtoblock;
  sigaddset(&sigtoblock, SIGTSTP);
  sigprocmask(SIG_BLOCK, &sigtoblock, NULL);


  // If background and no redirect, redirect to dev/null per assignment specifications
  if (!outFile && *isBackground) {
    outFile = "/dev/null";
  }
  
  if (!inFile && *isBackground) {
    inFile = "/dev/null";
  }

  // fork a new process
  pid_t spawnpid = fork();
  
  switch (spawnpid) {
    case -1:
      perror("Fork() failed");
      exit(1);
      break;
    case 0:

      // Install default behavior of SIGINT for foreground process
      if (*isBackground == 0) {
        struct sigaction sigint = {0};
        sigint.sa_handler = SIG_DFL;
        sigaction(SIGINT, &sigint, NULL);
      }


      // set up redirections, basically same code as in Module 5.
      if (outFile) {
        int outFD = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (outFD == -1) {
          printf("cannot open %s for output\n", outFile);
          fflush(stdout);
          exit(1);
        }
        
        // redirect stdout
        int result = dup2(outFD, 1);
        if (result == -1) {
          perror("target dup2()");
          exit(1);
        }
        // Set so child closes fd
        fcntl(outFD, F_SETFD, FD_CLOEXEC);
      }

      // Open input for redirect
      if (inFile) {
        int inFD = open(inFile, O_RDONLY);
        if (inFD == -1) {
          printf("cannot open %s for input\n", inFile);
          fflush(stdout);
          exit(1);
        }

        // Redirect stdin
        int result = dup2(inFD, 0);
        if (result == -1) {
          perror("target dup2()");
          exit(1);
        }

         fcntl(inFD, F_SETFD, FD_CLOEXEC);
      }
      
      // Execute the command, execvp process args until NULL is reached
      execvp(args[0], args);
      perror(args[0]);
      exit(1);
      
      break;

    default:
      if (*isBackground) {
        // WNOHANG flag for background process
        waitpid(spawnpid, exitStatus, WNOHANG);
        printf("background pid is %d\n", spawnpid);
        fflush(stdout);

      } else {
        // Forground process, wait for competion and set exit status
        waitpid(spawnpid, exitStatus, 0);
        if (WIFSIGNALED(*exitStatus)) {
          printf("terminated with signal %d\n", WTERMSIG(*exitStatus));
          fflush(stdout);
        } 
      }
      
      // Unblock the SIGTSTP signal 
      sigprocmask(SIG_UNBLOCK, &sigtoblock, NULL);
  }
}

/* Customer handler for the SIGTSTP signal
 * Code modified from class Module 5
 */
void handleSIGTSTP(int signo) {
  if (allowBG) {
    char *msgEnter = "\nEntering foreground-only mode (& is now ignored)\n";
    write(STDOUT_FILENO, msgEnter, 51);
    allowBG = 0;
  } else {
    char *msgExit = "\nExiting foreground-only mode\n";
    write(STDOUT_FILENO, msgExit, 31);
    allowBG = 1;
  }
  fflush(stdout);
}
