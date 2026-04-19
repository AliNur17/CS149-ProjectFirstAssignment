#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "fs_state.h"

void createF(char *spec, char *args)
{
    char normalized[MAX_PATH];
    int isDirectory;

    if (args == NULL || args[0] == '\0') {
        printf("usage: createF <file>\n");
        printf("       createF -mkdir <path>\n");
        return;
    }

    if (!fsNormalizePath(args, normalized, sizeof(normalized))) {
        printf("createF: invalid path '%s'\n", args);
        return;
    }

    if (spec != NULL && (strcmp(spec, "-mkdir") == 0 || strcmp(spec, "-mkDir") == 0)) {
        if (!fsIsDirectoryPath(normalized)) {
            printf("createF: cannot create directory '%s': final path component must not be a file\n", normalized);
            return;
        }

        if (fsFindByPath(normalized) != NULL) {
            printf("createF: cannot create '%s': File exists\n", normalized);
            return;
        }

        if (!fsCreateEntry(normalized, 1)) {
            printf("createF: failed to create '%s'\n", normalized);
            return;
        }

        printf("created directory '%s'\n", normalized);
        return;
    }

    isDirectory = fsIsDirectoryPath(normalized);
    if (isDirectory) {
        printf("createF: '%s' looks like a directory path. Use createF -mkdir <path>\n", normalized);
        return;
    }

    if (fsFindByPath(normalized) != NULL) {
        printf("createF: cannot create '%s': File exists\n", normalized);
        return;
    }

    if (!fsCreateEntry(normalized, 0)) {
        printf("createF: failed to create '%s'\n", normalized);
        return;
    }

    printf("created file '%s'\n", normalized);
}
