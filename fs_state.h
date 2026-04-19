#ifndef FS_STATE_H
#define FS_STATE_H

#include <stddef.h>

#define MAX_NAME 128
#define MAX_PATH 256
#define MAX_CONTENT 4096
#define FS_MAX_ENTRIES 256

typedef struct FileRecord {
    char name[MAX_NAME];
    char path[MAX_PATH];
    char content[MAX_CONTENT];
    int inode;
    int size;
    int isDirectory;
} FileRecord;

typedef struct {
    char name[MAX_NAME];
    char path[MAX_PATH];
    int isDirectory;
    int size;
} FsEntryView;

/* path/name helpers */
int fsExtractFileName(const char *path, char *outName, size_t outSize);
int fsNormalizePath(const char *input, char *outPath, size_t outSize);
int fsIsDirectoryPath(const char *path);

/* filesystem lookup/content helpers */
int fsGetEntries(FsEntryView entries[], int maxEntries);
const char *fsGetPathByName(const char *name);
const char *fsGetContentByName(const char *name);
int fsSetContentByName(const char *name, const char *content);
FileRecord *fsFindByPath(const char *path);
FileRecord *fsFindByName(const char *name);
int fsCreateEntry(const char *path, int isDirectory);

#endif
