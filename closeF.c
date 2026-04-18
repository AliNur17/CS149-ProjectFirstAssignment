#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands.h"
#include "fs_state.h"

void closeF(char *spec, char *args)
{
    if (strcmp(spec, "-list") == 0) {
        listOpenFiles();
        return;
    }

    int fd = atoi(args);

    for (int i = 0; i < fileCount; i++) {
        if (files[i].fd == fd && files[i].isOpen) {
            files[i].isOpen = 0;
            files[i].fd = -1;
            printf("Closed FD %d\n", fd);
            return;
        }
    }

    printf("Invalid file descriptor.\n");
}
