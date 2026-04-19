#include <stdio.h>
#include <string.h>
#include "fs_state.h"

File files[MAX_FILES];
int fileCount = 0;

File* findFile(const char *name) {
    for (int i = 0; i < fileCount; i++) {
        if (strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return NULL;
}

void listOpenFiles() {
    for (int i = 0; i < fileCount; i++) {
        if (files[i].isOpen) {
            printf("FD %d: %s\n", files[i].fd, files[i].name);
        }
    }
}

void listClosedFiles() {
    for (int i = 0; i < fileCount; i++) {
        if (!files[i].isOpen) {
            printf("%s\n", files[i].name);
        }
    }
}

void listAllFiles() {
    for (int i = 0; i < fileCount; i++) {
        printf("%s -> %s\n", files[i].name, files[i].path);
    }
}
