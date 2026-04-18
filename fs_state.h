#ifndef FS_STATE_H
#define FS_STATE_H

#define MAX_FILES 100
#define MAX_NAME 100
#define MAX_PATH 256

typedef struct {
    char name[MAX_NAME];
    char path[MAX_PATH];
    int isOpen;
    int fd;
    int canRead;
    int canWrite;
    char content[1024];
} File;

extern File files[MAX_FILES];
extern int fileCount;
extern int nextFD;

File* findFile(const char *name);
void listOpenFiles();
void listClosedFiles();
void listAllFiles();

#endif
