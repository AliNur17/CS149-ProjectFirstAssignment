#ifndef FS_STATE_H
#define FS_STATE_H

#include <stddef.h>

#define MAX_NAME 128
#define MAX_PATH 256
#define MAX_CONTENT 4096
#define FS_MAX_ENTRIES 256

typedef struct {
    char name[MAX_NAME];
    char path[MAX_PATH];
    int isDirectory;
    int size;
} FsEntryView;

/* path/name helpers */
int fsExtractFileName(const char *path, char *outName, size_t outSize);

/* filesystem lookup/content helpers */
int fsGetEntries(FsEntryView entries[], int maxEntries);
const char *fsGetPathByName(const char *name);
const char *fsGetContentByName(const char *name);
int fsSetContentByName(const char *name, const char *content);

#endif
