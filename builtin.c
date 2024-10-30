#include <assert.h>
#include <linux/limits.h>
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
#include <sys/time.h>
#include <time.h>
#include <utime.h>

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
    if (argpc > 1) {
        fprintf(stderr, "pwd: requires no arguments");
        return;
    }


    size_t path_sz = 4096;
    char path[path_sz];
    if (!getcwd(path, path_sz)) {
        perror("getcwd");
        return;
    }

    printf("%s\n", path);
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


static void cmdB_stat(char** args, int argc) {
    if (argc < 2) {
        fprintf(stderr, "stat: not enough arguments\n");
        return;
    }


    for (size_t i=1; i < argc; i++) {
        char *filename = args[i];
        struct stat file_info;
        if(stat(filename, &file_info) == -1) {
            perror("stat");
            return;
        }

        struct group group_info = *getgrgid(file_info.st_gid);
        struct passwd user_info = *getpwuid(file_info.st_uid);
        int file_mode = file_info.st_mode;


        char *file_type;
        if      (S_ISREG(file_mode))    file_type = "regular file";
        else if (S_ISDIR(file_mode))    file_type = "directory";
        else if (S_ISLNK(file_mode))    file_type = "symbolic link";
        else if (S_ISCHR(file_mode))    file_type = "character special file";
        else if (S_ISBLK(file_mode))    file_type = "block special file";
        else if (S_ISFIFO(file_mode))   file_type = "FIFO special file";
        else {
            fprintf(stderr, "stat: unknown filetype\n");
            return;
        }


        int perms = file_mode&0777;
        char perms_h[11];
        perms_h[0] = (S_ISDIR(file_mode)) ? 'd' : '-';
        perms_h[1] = (S_IRUSR & perms)    ? 'r' : '-';
        perms_h[2] = (S_IWUSR & perms)    ? 'w' : '-';
        perms_h[3] = (S_IXUSR & perms)    ? 'x' : '-';
        perms_h[4] = (S_IRGRP & perms)    ? 'r' : '-';
        perms_h[5] = (S_IWGRP & perms)    ? 'w' : '-';
        perms_h[6] = (S_IXGRP & perms)    ? 'x' : '-';
        perms_h[7] = (S_IROTH & perms)    ? 'r' : '-';
        perms_h[8] = (S_IWOTH & perms)    ? 'w' : '-';
        perms_h[9] = (S_IXOTH & perms)    ? 'x' : '-';


        struct tm *atime = localtime(&file_info.st_atime);
        struct tm *mtime = localtime(&file_info.st_mtime);
        struct tm *ctime = localtime(&file_info.st_ctime);

        char atime_h[20];
        char mtime_h[20];
        char ctime_h[20];
        char *format = "%Y-%m-%d %T";
        strftime(atime_h, 20, format, atime);
        strftime(mtime_h, 20, format, mtime);
        strftime(ctime_h, 20, format, ctime);


        printf("  File: %s\n"
               "  Size: %lu \tBlocks: %lu \tIO Block: %lu \t%s\n"
               "Device: %lu,%lu \tInode: %lu \tLinks: %lu\n"
               "Access: ( %04o/%s ) \tUid: ( %d/ %s ) \tGid: ( %d/ %s )\n"
               "Access: %s\n"
               "Modify: %s\n"
               "Change: %s\n",
               filename,
               file_info.st_size, file_info.st_blocks, file_info.st_blksize, file_type,
               file_info.st_rdev, file_info.st_dev, file_info.st_ino, file_info.st_nlink,
               perms, perms_h, file_info.st_uid, user_info.pw_name, file_info.st_gid, group_info.gr_name,
               atime_h,
               mtime_h,
               ctime_h);
    }

    return;
}


static void cmdB_tail(char** args, int argc) {
    if (argc < 2) {
        fprintf(stderr, "tail: not enough arguments\n");
        return;
    }


    // iterate over every file
    size_t total_lines;
    for (size_t i=1; i < argc; i++) {
        char *filename = args[i];
        FILE *fp = fopen(filename, "r");
        if (!fp) {
            perror("fopen");
            return;
        }


        total_lines = 0;
        while (!feof(fp)) {
            if (fgetc(fp) == '\n') total_lines++;
        }

        size_t tail_end = ftell(fp);
        size_t num_lines = (total_lines >= 10) ? 10 : total_lines;
        size_t start_line = total_lines - num_lines;

        if (fseek(fp, 0, SEEK_SET)) {
            perror("fseek");
            return;
        }

        // seek to start line
        size_t cur_line = 0;
        while (cur_line < start_line) {
            if (fgetc(fp) == '\n') cur_line++;
        }

        size_t tail_start = ftell(fp);
        size_t size = tail_end-tail_start;

        char buf[size+1];
        fread(buf, sizeof(char), size, fp);
        if (argc > 2) printf("\n==> %s <==\n", filename);
        fwrite(buf, sizeof(char), size, stdout);

        fclose(fp);
    }

    return;
}


static void cmdB_touch(char** args, int argc) {
    if (argc < 2) {
        fprintf(stderr, "touch: not enough arguments\n");
        return;
    }


    for (size_t i=0; i < argc; i++) {
        char *filename = args[i];
        FILE *fp = fopen(filename, "a");
        if (!fp) {
            perror("fopen");
            return;
        }

        if (utime(filename, NULL) == -1) perror("utime");
        fclose(fp);
    }

    return;
}
