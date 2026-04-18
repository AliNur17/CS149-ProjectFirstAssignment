#include <stdio.h>
#include <string.h>
#include "commands.h"

#define MAX_NAME 128
#define MAX_OPEN 32

typedef enum {
    TREE_ALPHA = 0,
    TREE_NUM   = 1,
    TREE_SYM   = 2
} TreeType;

typedef struct FileRecord {
    char name[MAX_NAME];
    int inode;
    int size;
} FileRecord;

typedef struct {
    int used;
    int fd;
    char name[MAX_NAME];
    char mode[4];
} OpenFile;

extern OpenFile openTable[MAX_OPEN];
extern int nextFD;
extern FileRecord *searchCentral(const char *name, TreeType *outType);

void openF(char *spec, char *args)
{
    const char *mode;
    FileRecord *file;
    int i;

    if (args == NULL || strlen(args) == 0) {
        printf("usage: openF [-r|-w|-rw] <file>\n");
        return;
    }

    if (strlen(spec) == 0 || strcmp(spec, "-r") == 0)
        mode = "r";
    else if (strcmp(spec, "-w") == 0)
        mode = "w";
    else if (strcmp(spec, "-rw") == 0)
        mode = "rw";
    else {
        printf("openF: invalid option '%s'\n", spec);
        return;
    }

    file = searchCentral(args, NULL);
    if (file == NULL) {
        printf("openF: cannot open '%s': No such file\n", args);
        return;
    }

    for (i = 0; i < MAX_OPEN; i++) {
        if (!openTable[i].used) {
            openTable[i].used = 1;
            openTable[i].fd = nextFD++;
            strncpy(openTable[i].name, args, MAX_NAME - 1);
            openTable[i].name[MAX_NAME - 1] = '\0';
            strncpy(openTable[i].mode, mode, 3);
            openTable[i].mode[3] = '\0';

            printf("opened '%s' as fd %d (%s)\n",
                   args, openTable[i].fd, openTable[i].mode);
            return;
        }
    }

    printf("openF: too many open files\n");
}
