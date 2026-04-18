#include <stdio.h>
#include "commands.h"

void helpList(char *spec, char *args)
{
    (void)spec;
    (void)args;

    printf("FILESYS(1)\n");
    printf("\n");
    printf("NAME\n");
    printf("    createF, openF, closeF, searchF, help - simulated file management commands\n");
    printf("\n");
    printf("SYNOPSIS\n");
    printf("    createF <file>\n");
    printf("    createF -mkdir <path>\n");
    printf("    openF [-r | -w | -rw] <file>\n");
    printf("    openF -list\n");
    printf("    openF -edit <file>\n");
    printf("    closeF <fd>\n");
    printf("    closeF -list\n");
    printf("    searchF <file>\n");
    printf("    searchF -dirL\n");
    printf("    help\n");
    printf("\n");
    printf("DESCRIPTION\n");
    printf("    createF\n");
    printf("        Create a file if it does not already exist.\n");
    printf("\n");
    printf("    createF -mkdir\n");
    printf("        Create a new directory at the specified path.\n");
    printf("        The path must begin with '\\\\' and use '\\\\' as the separator.\n");
    printf("        The final path component must be a directory name, not a file.\n");
    printf("\n");
    printf("    openF\n");
    printf("        Open a file in read, write, or read/write mode.\n");
    printf("\n");
    printf("    openF -list\n");
    printf("        Display all currently closed files that may be opened.\n");
    printf("\n");
    printf("    openF -edit\n");
    printf("        Open a file in a simple vi-like text editing mode.\n");
    printf("\n");
    printf("    closeF\n");
    printf("        Close an active file descriptor.\n");
    printf("\n");
    printf("    closeF -list\n");
    printf("        Display all currently open files.\n");
    printf("\n");
    printf("    searchF\n");
    printf("        Search the indexed file system for a file and print its full path.\n");
    printf("\n");
    printf("    searchF -dirL\n");
    printf("        List all files line by line, including folders and subfolders.\n");
    printf("\n");
    printf("    help\n");
    printf("        Display this help page.\n");
    printf("\n");
    printf("EXAMPLES\n");
    printf("    createF report.txt\n");
    printf("    createF -mkdir \\\\docs\\\\projects\n");
    printf("    openF -rw report.txt\n");
    printf("    openF -edit notes.txt\n");
    printf("    openF -list\n");
    printf("    searchF report.txt\n");
    printf("    searchF -dirL\n");
    printf("    closeF 3\n");
    printf("    closeF -list\n");
    printf("\n");
    printf("NOTES\n");
    printf("    This is a simulated file management environment and does not operate\n");
    printf("    directly on the host operating system's real file system.\n");
}
