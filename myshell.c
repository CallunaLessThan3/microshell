/* CS 347 -- Mini Shell!
 * Original author Phil Nelson 2000
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include "argparse.h"
#include "builtin.h"
#include <malloc.h>

#define DEFAULT_BUFSIZE (1)

/* PROTOTYPES */

void processline (char *line);
ssize_t getinput(char** line, size_t* size);


/*
* main
*
* Main entry point of the myshell program.
* This is essentially the primary read-eval-print loop of the command interpreter.
*
* Runs the shell in an endless loop until an exit command is issued.
*
* Hint: Use getinput and processline as appropriate.
*/
int main () {
    size_t size = 0;
    char *line = malloc(DEFAULT_BUFSIZE);
    int argcp;

    ssize_t length = getinput(&line, &size);
    processline(line);

    /*
    char **args = argparse(line, &argcp);

    printf("   read: %s\n", line);
    printf(" length: %lu\n", length);
    printf("bufsize: %lu\n", size);
    for (size_t i=0; i<argcp; i++) {
        printf("  arg %lu: %s\n", i, args[i]);
    }
    */


    free(line);
    return EXIT_SUCCESS;
}


/*
* getinput
*
* Prompts the user for a line of input (e.g. %myshell%) and stores it in a dynamically
* allocated buffer (pointed to by *line).
* If input fits in the buffer, it is stored in *line.
* If the buffer is too small, *line is freed and a larger buffer is allocated.
* The size of the buffer is stored in *size.
*
* Args:
*   line: pointer to a char* that will be set to the address of the input buffer
*   size: pointer to a size_t that will be set to the size of the buffer *line or 0 if *line is NULL.
*
* Returns:
*   The length of the the string stored in *line.
*
* Hint: There is a standard i/o function that can make getinput easier than it sounds.
*/
ssize_t getinput(char** line, size_t* size) {
    size_t bufsize = DEFAULT_BUFSIZE;
    size_t cursize = 0;
    char *runner = *line;

    char ch;
    printf("%% ");

    while ((ch = getchar()) != '\n') {
        // +1 is size including null terminator
        if ((cursize+1) >= bufsize) {
            bufsize *= 2;
            *line = realloc(*line, bufsize);
            runner = *line+cursize;
        }
        *runner = ch;
        runner++;
        cursize++;
    }
    *runner = '\0';

    *size = bufsize;
    return cursize;
}


/*
* processline
*
* Interprets the input line as a command and either executes it as a built-in
* or forks a child process to execute an external program.
* Built-in commands are executed immediately.
* External commands are parsed then forked to be executed.
*
* Args:
*   line: string containing a shell command and arguments
*
* Note: There are three cases to consider when forking a child process:
*   1. Fork fails
*   2. Fork succeeds and this is the child process
*   3. Fork succeeds and this is the parent process
*
* Hint: See the man page for fork(2) for more information.
* Hint: The process should only fork when the line is not empty and not trying to
*       run a built-in command.
*/
void processline (char *line) {
    /* check whether line is empty */
    if (strlen(line) < 1) { return; }

    pid_t cpid;
    int status;
    int argCount;
    char** arguments = argparse(line, &argCount);

    /* check whether arguments are builtin commands
       if not builtin, fork to execute the command. */
    if (builtIn(arguments, argCount)) { return; } //TODO: implement
    else {
        cpid = fork();
        if (cpid == -1) { perror("fork()"); }
        //child
        if (cpid == 0) {
            if (execvp(*arguments, arguments) == -1) { perror("execvp()"); }
            return;
        //parent
        } else {
            wait(&status);
        }
    }
}

