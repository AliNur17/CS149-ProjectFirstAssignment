#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commands.h"
#include "openF_helpers.h"

void closeF(char *spec, char *args)
{
    int fd;
    int i;
    int found = 0;

    if (spec != NULL && strcmp(spec, "-list") == 0) {
        printf("Open files:\n");
        for (i = 0; i < MAX_OPEN; i++) {
            if (openTable[i].used) {
                printf("  fd %d  %s  (%s)\n",
                       openTable[i].fd,
                       openTable[i].name,
                       openTable[i].mode);
                found = 1;
            }
        }

        if (!found)
            printf("  (none)\n");
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
            printf("closed fd %d\n", fd);
            return;
        }
    }

    printf("closeF: invalid file descriptor '%s'\n", args);
}
