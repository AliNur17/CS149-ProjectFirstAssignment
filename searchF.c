#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "fs_state.h"

void searchF(char *spec, char *args)
{
    if (strcmp(spec, "-dirL") == 0) {
        listAllFiles();
        return;
    }

    File *f = searchCentral(args);

    if (!f) {
        printf("File not found.\n");
        return;
    }

    printf("Found: %s\nPath: %s\n", f->name, f->path);
}
