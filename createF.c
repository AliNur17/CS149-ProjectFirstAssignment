#include <stdio.h>
#include <string.h>
#include "commands.h"

#define MAX_NAME 128

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

extern FileRecord *searchCentral(const char *name, TreeType *outType);
extern int insertFileCentral(const char *name);

void createF(char *spec, char *args)
{
    TreeType type;
    FileRecord *file;

    (void)spec;

    if (args == NULL || strlen(args) == 0) {
        printf("usage: createF <file>\n");
        return;
    }

    file = searchCentral(args, &type);
    if (file != NULL) {
        printf("createF: cannot create '%s': File exists\n", args);
        return;
    }

    if (!insertFileCentral(args)) {
        printf("createF: failed to create '%s'\n", args);
        return;
    }

    printf("created '%s'\n", args);
}
