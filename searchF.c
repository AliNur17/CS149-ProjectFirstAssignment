#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "fs_state.h"
#include "openF_helpers.h"

void searchF(char *spec, char *args)
{
    char normalized[MAX_PATH];
    FileRecord *f;

    if (spec != NULL && (strcmp(spec, "-dirL") == 0 || strcmp(spec, "-dirl") == 0)) {
        FsEntryView entries[FS_MAX_ENTRIES];
        int count;
        int i;

        count = fsGetEntries(entries, FS_MAX_ENTRIES);

        if (count == 0) {
            printf("(empty)\n");
            return;
        }

        for (i = 0; i < count; i++) {
            printf("%s%s\n", entries[i].path, entries[i].isDirectory ? "/" : "");
        }
        return;
    }

    if (args == NULL || args[0] == '\0') {
        printf("usage: searchF <file>\n");
        return;
    }

    if (!fsNormalizePath(args, normalized, sizeof(normalized))) {
        printf("searchF: invalid path '%s'\n", args);
        return;
    }

    f = fsFindByPath(normalized);
    if (f == NULL && strchr(args, '/') == NULL && strchr(args, '\\') == NULL)
        f = searchCentral(args, NULL);

    if (f == NULL) {
        printf("searchF: '%s' not found\n", args);
        return;
    }

    if (f->isDirectory)
        printf("directory: %s\n", f->path);
    else
        printf("file: %s\n", f->path);
}
