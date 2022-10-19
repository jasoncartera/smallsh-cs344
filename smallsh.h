#ifndef SMALLSH
#define SMALLSH
#include <unistd.h>

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

void parseInput(char*[], pid_t, int*, int*, char**, char**);

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

void runCommand(char*[], int*, int*, char*, char*);

/* Customer handler for the SIGTSTP signal
 * Code modified from class Module 5
 */
void handleSIGTSTP(int signo);

#endif 
