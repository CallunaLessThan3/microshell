#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include "builtin.h"

//Prototypes
static void exitProgram(char** args, int argcp);
static void cd(char** args, int argpcp);
static void pwd(char** args, int argcp);

//Group A
static void cmdA_ls(char** args, int argc);
static void cmdA_cp(char** args, int argc);
static void cmdA_env(char** args, int argc);

//Group B
static void cmdB_stat(char** args, int argc);
static void cmdB_tail(char** args, int argc);
static void cmdB_touch(char** args, int argc);

#define TRUE  (1)
#define FALSE (0)

/*
* builtIn
*
* Checks if a command is a built-in shell command and executes it if so.
*
* Args:
*   args: array of strings containing command and arguments
*   argcp: number of elements in args array
*
* Returns:
*   1 if the command was a built-in, 0 otherwise.
*
* Built-in commands are executed directly by the shell process rather than
* being forked to a new process. This function compares the given command
* to each of the built-ins (exit, pwd, cd, and ls/cp/env or stat/tail/touch
* depending on group). If a match is found, the corresponding function is called.
*
* Hint: Refer to checklist for group specific examples
*/
int builtIn(char** args, int argcp) {
    //TEMPORARY: consider using struct or #defines and switch statement
    int builtin;
    if (!strcmp(*args, "exit")) {
        exitProgram(args, argcp);
        builtin = TRUE;
    }
    else if (!strcmp(*args, "cd")) {
        cd(args, argcp);
        builtin = TRUE;
    }
    else if (!strcmp(*args, "pwd")) {
        pwd(args, argcp);
        builtin = TRUE;
    }
    else if (!strcmp(*args, "stat")) {
        cmdB_stat(args, argcp);
        builtin = TRUE;
    }
    else if (!strcmp(*args, "tail")) {
        cmdB_tail(args, argcp);
        builtin = TRUE;
    }
    else if (!strcmp(*args, "touch")) {
        cmdB_touch(args, argcp);
        builtin = TRUE;
    }
    else {
        builtin = FALSE;
    }

    return builtin;
}

/*
* exitProgram
*
* Terminates the shell with a given exit status.
* If no exit status is provided, exits with status 0.
* This function should use the exit(3) library call.
*
* Args:
*   args: array of strings containing "exit" and optionally an exit status
*   argcp: number of elements in args array
*/
static void exitProgram(char** args, int argcp) {
    int status = 0;
    if (argcp == 2) {
        status = atoi(args[1]);
    } else if (argcp > 2) {
        fprintf(stderr, "exit: too many arguments\n");
        return;
    }

    exit(status);
    return;
}

/*
* pwd
*
* Prints the current working directory.
*
* Args:
*   args: array of strings containing "pwd"
*   argcp: number of elements in args array, should be 1
*
* Example Usage:
*   Command: $ pwd
*   Output: /some/path/to/directory
*/
static void pwd(char** args, int argpc) {
    return;
}

/*
* cd
*
* Changes the current working directory.
* When no parameters are provided, changes to the home directory.
* Supports . (current directory) and .. (parent directory).
*
* Args:
*   args: array of strings containing "cd" and optionally a directory path
*   argcp: number of elements in args array
*
* Example Usage:
*   Command: $ pwd
*   Output: /some/path/to/directory
*   Command: $ cd ..
*   Command: $ pwd
*   Output: /some/path/to
*
* Hint: Read the man page for chdir(2)
*/
static void cd(char** args, int argcp) {
    return;
}


/*
* fileType
*
* Helper for cmdB_stat.
* Sets file_type to file type of given file
*
* Args:
*   file: stat struct for file
*   file_type: string stating file type
*/
static void fileType(struct stat* file, char** file_type) {
    int file_mode = (*file).st_mode;
    if      (S_ISREG(file_mode))    { *file_type = "regular file";           }
    else if (S_ISDIR(file_mode))    { *file_type = "directory";              }
    else if (S_ISLNK(file_mode))    { *file_type = "symbolic link";          }
    else if (S_ISCHR(file_mode))    { *file_type = "character special file"; }
    else if (S_ISBLK(file_mode))    { *file_type = "block special file";     }
    else if (S_ISFIFO(file_mode))   { *file_type = "FIFO special file";      }
    else                            { *file_type = "unknown";                }
    return;
}


static void cmdB_stat(char** args, int argc) {
    struct stat file_info;
    int status = stat(args[1], &file_info);
    if (status == -1) {
        perror("stat");
        return;
    }

    struct group group_info = *getgrgid(file_info.st_gid);
    struct passwd user_info = *getpwuid(file_info.st_uid);

    char *file_type;
    fileType(&file_info, &file_type);

    //TODO: maybe extract to diff function
    char perms_h[10];
    int perms = file_info.st_mode&0777;
    perms_h[0] = (S_ISDIR(file_info.st_mode)) ? 'd' : '-';
    perms_h[1] = (S_IRUSR & perms) ? 'r' : '-';
    perms_h[2] = (S_IWUSR & perms) ? 'w' : '-';
    perms_h[3] = (S_IXUSR & perms) ? 'x' : '-';
    perms_h[4] = (S_IRGRP & perms) ? 'r' : '-';
    perms_h[5] = (S_IWGRP & perms) ? 'w' : '-';
    perms_h[6] = (S_IXGRP & perms) ? 'x' : '-';
    perms_h[7] = (S_IROTH & perms) ? 'r' : '-';
    perms_h[8] = (S_IWOTH & perms) ? 'w' : '-';
    perms_h[9] = (S_IXOTH & perms) ? 'x' : '-';


    //TODO: set time to something readable
    printf("  File: %s\n"
           "  Size: %lu \tBlocks: %lu \tIO Block: %lu \t%s\n"
           "Device: %lu,%lu \tInode: %lu \tLinks: %lu\n"
           "Access: ( %04o/%s ) \tUid: ( %d/ %s ) \tGid: ( %d/ %s )\n"
           "Access: %lu\n"
           "Modify: %lu\n"
           "Change: %lu\n",
           args[1],
           file_info.st_size, file_info.st_blocks, file_info.st_blksize, file_type,
           file_info.st_rdev, file_info.st_dev, file_info.st_ino, file_info.st_nlink,
           perms, perms_h, file_info.st_uid, user_info.pw_name, file_info.st_gid, group_info.gr_name,
           file_info.st_atime,
           file_info.st_mtime,
           file_info.st_ctime);
    return;
}


static void cmdB_tail(char** args, int argc) {
    return;
}


static void cmdB_touch(char** args, int argc) {
    return;
}
