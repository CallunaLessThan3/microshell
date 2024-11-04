#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "argparse.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define FALSE (0)
#define TRUE  (1)

/*
* argCount
*
* Counts the number of arguments in a given input line.
* You may assume only whitespace is used to separate arguments.
* argCount should be able to handle multiple whitespaces between arguments.
*
* Args:
*   line: input string containing command and arguments separated by whitespaces
*
* Returns:
*   The number of arguments in line.
*   (The command itself counts as the first argument)
*
* Example:
*   argCount("ls -l /home") returns 3
*   argCount("   ls    -l   /home  ") returns 3
*/
static int argCount(char* line) {
    /* LOGIC
    if: !word and *runner is !space:
        => word = TRUE;
           args++;
    if: word and *runner is space:
        => word = FALSE;
    */
    size_t args = 0;
    size_t word = FALSE;
    char *runner = line;

    while (*runner) {
        if (!word && !isspace(*runner)) {
            word = TRUE;
            args++;
        } else if (word && isspace(*runner)) {
            word = FALSE;
        }
        runner++;
    }

    return args;
}



/*
* argparse
*
* Parses an input line into an array of strings.
*
*
* You may assume only whitespace is used to separate strings.
* argparse should be able to handle multiple whitespaces between strings.
* The function should dynamically allocate space for the array of strings,
* following the project requirements. 
*
* Args:
*   line: input string containing words separated by whitespace
*   argcp: stores the number of strings in the line
*
* Returns:
*   An array of strings.
*
* Example:
*   argparse("ls -l /home", &argc) --> returns ["ls", "-l", "/home"] and set argc to 3
*   argparse("   ls    -l   /home  ", &argc) --> returns ["ls", "-l", "/home"] and set argc to 3
*/
//all of this feels bad ;w;
char** argparse(char* line, int* argcp) {
    *argcp = argCount(line);
    // Memory leak here
    char **ret = calloc((*argcp)+1, sizeof(char *));
    char **ret_runner = ret;
    char *runner = line;

    size_t cur_arg = 0;
    while (cur_arg < *argcp) {
        int word_len = 0;
        char *word_start = runner;
        //could use 2 runners: start, end => end-start = word_len
        while (*runner && !isspace(*runner)) {
            runner++;
            word_len++;
        }

        if (word_len > 0) {
            *ret_runner = calloc(word_len+1, sizeof(char));
            // Another memory leak here
            memcpy(*ret_runner, word_start, word_len);
            ret_runner++;
            cur_arg++;
        }
        runner++;
    }

    return ret;
}
