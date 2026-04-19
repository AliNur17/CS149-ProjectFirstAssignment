#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands.h"
#include "openF_helpers.h"

void closeF(char *spec, char *args)
{
    int fd;
    int i;

    if (spec != NULL && strcmp(spec, "-list") == 0) {
        for (i = 0; i < MAX_OPEN; i++) {
            if (openTable[i].used) {
                printf("FD %d: %s (%s)\n",
                       openTable[i].fd,
                       openTable[i].name,
                       openTable[i].mode);
            }
        }
        return;
    }

    if (args == NULL || args[0] == '\0') {
        printf("usage: closeF <fd>\n");
        return;
    }

    fd = atoi(args);

    for (i = 0; i < MAX_OPEN; i++) {
        if (openTable[i].used && openTable[i].fd == fd) {
            openTable[i].used = 0;
            printf("Closed FD %d\n", fd);
            return;
        }
    }

    printf("Invalid file descriptor.\n");
}
