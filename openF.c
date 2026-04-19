#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "fs_state.h"
#include "openF_helpers.h"

static FileRecord *resolveFileTarget(const char *arg, char *normalized, size_t normalizedSize)
{
    FileRecord *file;

    if (!fsNormalizePath(arg, normalized, normalizedSize))
        return NULL;

    file = fsFindByPath(normalized);
    if (file != NULL)
        return file;

    if (strchr(arg, '/') == NULL && strchr(arg, '\\') == NULL)
        return fsFindByName(arg);

    return NULL;
}

void openF(char *spec, char *args)
{
    const char *mode;
    FileRecord *file;
    char normalized[MAX_PATH];
    int i;

    if (spec != NULL && strcmp(spec, "-list") == 0) {
        listClosedFiles();
        return;
    }

    if (spec != NULL && strcmp(spec, "-edit") == 0) {
        if (args == NULL || strlen(args) == 0) {
            printf("usage: openF -edit <file>\n");
            return;
        }

        file = resolveFileTarget(args, normalized, sizeof(normalized));
        if (file == NULL || file->isDirectory) {
            printf("openF: cannot open '%s': No such file\n", args);
            return;
        }

        editFile(file);
        return;
    }

    if (spec != NULL && strcmp(spec, "-content") == 0) {
        if (args == NULL || strlen(args) == 0) {
            printf("usage: openF -content <file>\n");
            return;
        }

        file = resolveFileTarget(args, normalized, sizeof(normalized));
        if (file == NULL || file->isDirectory) {
            printf("openF: cannot open '%s': No such file\n", args);
            return;
        }

        showFileContent(file);
        return;
    }

    if (args == NULL || strlen(args) == 0) {
        printf("usage: openF [-r|-w|-rw] <file>\n");
        return;
    }

    if (spec == NULL || strlen(spec) == 0 || strcmp(spec, "-r") == 0)
        mode = "r";
    else if (strcmp(spec, "-w") == 0)
        mode = "w";
    else if (strcmp(spec, "-rw") == 0)
        mode = "rw";
    else {
        printf("openF: invalid option '%s'\n", spec);
        return;
    }

    file = resolveFileTarget(args, normalized, sizeof(normalized));
    if (file == NULL || file->isDirectory) {
        printf("openF: cannot open '%s': No such file\n", args);
        return;
    }

    for (i = 0; i < MAX_OPEN; i++) {
        if (!openTable[i].used) {
            openTable[i].used = 1;
            openTable[i].fd = nextFD++;
            strncpy(openTable[i].name, file->name, MAX_NAME - 1);
            openTable[i].name[MAX_NAME - 1] = '\0';
            strncpy(openTable[i].mode, mode, 3);
            openTable[i].mode[3] = '\0';

            printf("opened '%s' as fd %d (%s)\n",
                   file->path, openTable[i].fd, openTable[i].mode);
            return;
        }
    }

    printf("openF: too many open files\n");
}
