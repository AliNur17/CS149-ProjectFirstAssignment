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

void searchF(char *spec, char *args)
{
    TreeType type;
    FileRecord *file;

    (void)spec;

    if (args == NULL || strlen(args) == 0) {
        printf("usage: searchF <file>\n");
        return;
    }

    file = searchCentral(args, &type);

    if (file == NULL) {
        printf("searchF: '%s' not found\n", args);
    } else {
        const char *treeName;

        if (type == TREE_ALPHA)
            treeName = "Alphabetic B+ Tree";
        else if (type == TREE_NUM)
            treeName = "Numeric B+ Tree";
        else
            treeName = "Symbol B+ Tree";

        printf("found '%s' [inode=%d, size=%d, index=%s]\n",
               file->name, file->inode, file->size, treeName);
    }
}
