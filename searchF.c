#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "fs_state.h"
#include "openF_helpers.h"

void searchF(char *spec, char *args)
{
    char fileName[MAX_NAME];
    FileRecord *f;

    if (spec != NULL && strcmp(spec, "-dirL") == 0) {
        FsEntryView entries[FS_MAX_ENTRIES];
        int count;
        int i;

        count = fsGetEntries(entries, FS_MAX_ENTRIES);

        for (i = 0; i < count; i++) {
            if (!entries[i].isDirectory)
                printf("%s -> %s\n", entries[i].name, entries[i].path);
        }
        return;
    }

    if (args == NULL || args[0] == '\0') {
        printf("usage: searchF <file>\n");
        return;
    }

    if (!fsExtractFileName(args, fileName, sizeof(fileName))) {
        printf("searchF: invalid file path '%s'\n", args);
        return;
    }

    f = searchCentral(fileName, NULL);

    if (f == NULL) {
        printf("File not found.\n");
        return;
    }

    printf("Found: %s\nPath: %s\n", f->name, f->path);
}
