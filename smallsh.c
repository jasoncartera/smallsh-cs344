#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


#define MAX_ARGS 512
#define MAX_IN 2048

void parseInput(char*[], pid_t, int*, int*, char*, char*);
void runCommand(char*[], int*, int*, char*, char*);

/* 
 * main() processes the input and executes commands
 */
int main(void) {
  
  // Initalize NULL to make execv call easier
  char *args[MAX_ARGS] = {NULL};
  pid_t pid = getpid();
  int exitStatus = 0;
    // Start main shell loop
  while(1) {
    
    // These vars are reset each loop
    int argc = 0;
    int isBackground = 0;
    pid_t childPid;
    char *inFile = malloc(strlen("")+1);
    char *outFile = malloc(strlen("")+1);
    strcpy(inFile, "");
    strcpy(outFile, "");

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
    parseInput(args, pid, &argc, &isBackground, inFile, outFile);
    // If comment or no input, continue
    if (args[0][0] == '#' || argc == 0) {
      continue;
    }
    
    // Process exit command
    else if (!strcmp(args[0], "exit")) {
      // Free memory first
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

    //Reset args array after each command
    for (int i = 0; i < MAX_ARGS; i++) {
      args[i] = NULL;
    }

  }
  return 0;
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
  

  // fork a new process
  pid_t spawnpid = fork();
  switch (spawnpid) {
    case -1:
      perror("Fork() failed");
      exit(1);
      break;
    case 0:
      // set up redirections, basically same code as in Module 5.
      if (strcmp(outFile, "")) {
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
        // Set so child closes
        fcntl(outFD, F_SETFD, FD_CLOEXEC);
      }

      // Open input for redirect
      if (strcmp(inFile, "")) {
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
      }
  }

}


/*
 * Prompts the user and parses input into an array of arguments
 *
 * params:
 *  args:          array of arguments
 *  pid:           shell pid
 *  argc:          number of arguments passed
 *  isBackground:  checks if command should be a background process or not
 *  inFile:        name of inFile if there is an input redirect
 *  outFile:       name of outFile if there is an output redirect
 */

void parseInput(char *args[], pid_t pid, int *argc, int *isBackground, char *inFile, char *outFile) {
  

  /// Max input length defined at 2048 chars
  char input[MAX_IN];

  printf(": ");
  fflush(stdout);
  fgets(input, MAX_IN, stdin);


  // Remove trailing new line from fgets and replace with \0
  input[strcspn(input, "\n")] = '\0';
  
  // Retrun if command is nothing
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
    *argc += 1;
    lastWord = realloc(lastWord, strlen(token)+1);
    strcpy(lastWord, token);
    // Check for output redirect
    if (strcmp(token, ">") == 0) {
      // Advance the token early to get file name
      token = strtok(NULL, " ");
      outFile = realloc(outFile, strlen(token)+1);
      strcpy(outFile, token);
    }
    // Check for input redirect 
    else if (strcmp(token, "<") == 0) {
      token = strtok(NULL, " ");
      inFile = realloc(inFile, strlen(token)+1);
      strcpy(inFile, token);
    }
    // add to args
    else {
      args[i] = malloc(strlen(token)+1);
      strcpy(args[i], token);
    }
    token = strtok(NULL, " ");
  }

  if (!strcmp(lastWord, "&")) {
    *isBackground = 1;
    // Remove flag so command will exec properly
    args[*argc-1] = NULL;
  }

  free(lastWord);

}


