#include <stdio.h>
#include "commands.h"

void helpList(char *spec, char *args)
{
    (void)spec;
    (void)args;

    printf("NAME\n");
    printf("    createF, openF, closeF, searchF - simulated file management commands\n");
    printf("\n");
    printf("SYNOPSIS\n");
    printf("    createF <file>\n");
    printf("    openF [-r | -w | -rw] <file>\n");
    printf("    closeF <fd>\n");
    printf("    searchF <file>\n");
    printf("    help\n");
    printf("\n");
    printf("DESCRIPTION\n");
    printf("    createF    Creates a file unless the file already exists.\n");
    printf("    openF      Opens a file in read, write, or read/write mode.\n");
    printf("    closeF     Closes an active file descriptor.\n");
    printf("    searchF    Searches the indexed file system for a file.\n");
    printf("    help       Displays command usage information.\n");
    printf("\n");
    printf("EXAMPLES\n");
    printf("    createF report.txt\n");
    printf("    openF -rw report.txt\n");
    printf("    searchF report.txt\n");
    printf("    closeF 3\n");
}
