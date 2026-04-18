#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

#define MAX_NAME 128
#define MAX_OPEN 32

typedef struct {
    int used;
    int fd;
    char name[MAX_NAME];
    char mode[4];
} OpenFile;

extern OpenFile openTable[MAX_OPEN];

void closeF(char *spec, char *args)
{
    int fd;
    int i;

    (void)spec;

    if (args == NULL || strlen(args) == 0) {
        printf("usage: closeF <fd>\n");
        return;
    }

    fd = atoi(args);

    for (i = 0; i < MAX_OPEN; i++) {
        if (openTable[i].used && openTable[i].fd == fd) {
            openTable[i].used = 0;
            printf("closed fd %d ('%s')\n", fd, openTable[i].name);
            return;
        }
    }

    printf("closeF: %d: Bad file descriptor\n", fd);
}
