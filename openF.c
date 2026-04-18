#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "fs_state.h"

void openF(char *spec, char *args)
{
    if (strcmp(spec, "-list") == 0) {
        listClosedFiles();
        return;
    }

    File *f = findFile(args);
    if (!f) {
        printf("File not found.\n");
        return;
    }

    if (strcmp(spec, "-edit") == 0) {
        if (!f->canWrite) {
            printf("No write permission.\n");
            return;
        }
        f->isOpen = 1;
        f->fd = nextFD++;
        printf("Editing %s (fd=%d)\n", args, f->fd);
        return;
    }

    if (strcmp(spec, "-content") == 0) {
        if (!f->canRead) {
            printf("No read permission.\n");
            return;
        }
        printf("Content of %s:\n%s\n", args, f->content);
        return;
    }

    if (strcmp(spec, "-r") == 0) {
        f->canRead = 1;
        f->canWrite = 0;
    } else if (strcmp(spec, "-w") == 0) {
        f->canRead = 0;
        f->canWrite = 1;
    } else {
        f->canRead = 1;
        f->canWrite = 1;
    }

    f->isOpen = 1;
    f->fd = nextFD++;

    printf("Opened %s (fd=%d)\n", args, f->fd);
}
