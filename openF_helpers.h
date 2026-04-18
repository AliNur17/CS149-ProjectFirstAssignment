#ifndef OPENF_HELPERS_H
#define OPENF_HELPERS_H

#define MAX_NAME 128
#define MAX_OPEN 32
#define MAX_EDIT_BUFFER 4096
#define MAX_EDIT_LINES 256
#define MAX_LINE_LENGTH 512

typedef enum {
    TREE_ALPHA = 0,
    TREE_NUM   = 1,
    TREE_SYM   = 2
} TreeType;

typedef struct FileRecord {
    char name[MAX_NAME];
    int inode;
    int size;
} FileRecord;

typedef struct {
    int used;
    int fd;
    char name[MAX_NAME];
    char mode[4];
} OpenFile;

extern OpenFile openTable[MAX_OPEN];
extern int nextFD;

extern FileRecord *searchCentral(const char *name, TreeType *outType);
extern void enableRawMode(void);
extern void disableRawMode(void);

int fileHasActiveReadableHandle(const char *name);
int fileHasActiveWritableHandle(const char *name);
int fileCurrentlyOpen(const char *name);
void listClosedFiles(void);
void showFileContent(FileRecord *file);
void editFile(FileRecord *file);

#endif
