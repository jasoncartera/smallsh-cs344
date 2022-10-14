#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


#define MAX_ARGS 512
#define MAX_IN 2048

void parseInput(char*[], pid_t, int*, int*, int*);
void runCommand(char*[], int*, int*);
/* 
 * main() processes the input and executes the appropraite function
 */
int main(void) {
  
  char *args[MAX_ARGS] = {NULL};
  pid_t pid = getppid();
  int exitStatus = 0;

  // Start loop
  while(1) {
    
    // Reset each loop
    int argc = 0;
    int isBackground = 0;
    

    // Parse the input
    parseInput(args, pid, &argc, &isBackground, &exitStatus);
    
    // If comment or no input continue
    if (args[0][0] == '#' || args[0][0] == '\0') {
      continue;
    }

    else if (!strcmp(args[0], "exit")) {
      exit(0);
    }
    
    else if (!strcmp(args[0], "cd")) {
      if (argc > 1) {
        if (chdir(args[1]) == -1) {
        } else {
          chdir(args[1]);
          }

      } else {
        chdir(getenv("HOME"));
      }
    }

    else if (!strcmp(args[0], "status")) {
        printf("%d\n", exitStatus);
    }
    
    // All other commands
    else {
      runCommand(args, &exitStatus, &isBackground);
    }

    //Reset input
    for (int i = 0; i < MAX_ARGS; i++) {
      args[i] = NULL;
    }
  }
}

/*
 * Runs commands other than exit, cd, and status
 */

void runCommand(char *args[], int *exitStatus, int *isBackground) {
  int childStatus;
  int childPid;
  
  pid_t spawnpid = fork();

  switch (spawnpid) {
    case -1:
      perror("Fork() failed");
      exit(1);
      break;
    case 0:
      execvp(args[0], args);
      perror(args[0]);
      exit(1);
      break;
    default:
      if (*isBackground) {
        pid_t waitPid = waitpid(spawnpid, exitStatus, WNOHANG);
        printf("background pid is %d\n", spawnpid);
        fflush(stdout);

      } else {
        pid_t waitPid = waitpid(spawnpid, exitStatus, 0);
          if (WIFEXITED(*exitStatus)) {
            *exitStatus = WEXITSTATUS(*exitStatus);
          }
          if (WIFSIGNALED(*exitStatus)) {
            *exitStatus = WTERMSIG(*exitStatus);
          }
        }
  }

}


/*
 * Prompts the user and parses input into an array of arguments
 */
void parseInput(char *args[], pid_t pid, int *argc, int *isBackground, int *exitStatus) {
  

  /// Max input length defined at 2048 chars
  char input[MAX_IN];
  pid_t childPid; 

  // Print out any completed background processes first
  while ((childPid = waitpid(-1, exitStatus, WNOHANG)) >0) { 
    if (WIFEXITED(*exitStatus)) {
          *exitStatus = WEXITSTATUS(*exitStatus);
        } 
        if (WIFSIGNALED(*exitStatus)) {
          *exitStatus = WTERMSIG(*exitStatus);
        }  
      printf("Background process %d terminated with status %d\n", childPid, *exitStatus);
      fflush(stdout);
   }
  
  printf(": ");
  fflush(stdout);
  fgets(input, MAX_IN, stdin);


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
  char *lastWord;
  for (int i = 0; token; i++) {
    args[i] = strdup(token); 
    *argc += 1;
    lastWord = strdup(token);
    token = strtok(NULL, " ");
  }

  if (!strcmp(lastWord, "&")) {
    *isBackground = 1;
    // Remove flag so command will exec properly
    args[*argc-1] = NULL;
  }
  free(lastWord);

}
