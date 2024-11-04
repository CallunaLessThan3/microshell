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


/* PROTOTYPES */
void processline (char *line);
ssize_t getinput(char** line, size_t* size);
void cleanup(void);

static char *line;
static char **args;
static int argcp;


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
    atexit(&cleanup);
    while (1) {
        size_t size;
        getinput(&line, &size);
        processline(line);
        cleanup();
    }

    return EXIT_SUCCESS;
}


/*
 * cleanup
 *
 * Frees allocated memory
*/
void cleanup(void) {
    for (size_t i=0; i < argcp; i++) {
        free(args[i]);
    }

    free(args);
    free(line);

    return;
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
    size_t bufsize = 2;
    size_t cursize = 0;
    *line = malloc(bufsize);
    char *runner = *line;

    printf("%% ");

    // Allocates enough space for stdin by doubling each time
    char ch;
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
    *size = cursize+1;

    // Allocates minimum amount of space for string
    *line = realloc(*line, *size);
    if(!(*line)) {
        perror("realloc");
        exit(-1);
    }

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
void processline(char *line) {
    /* Check whether line is empty */
    if (strlen(line) < 1) return;

    pid_t cpid;
    int status;
    int argCount;
    char** arguments = argparse(line, &argCount);

    /* Check whether arguments are builtin commands
       if not builtin, fork to execute the command. */
    if (builtIn(arguments, argCount)) return;
    else {
        cpid = fork();
        if (cpid == -1) perror("fork");
        // Child
        if (cpid == 0) {
            if (execvp(*arguments, arguments) == -1) perror("execvp");
            return;
        // Parent
        } else {
            free(*arguments);
            wait(&status);
        }
    }
}

