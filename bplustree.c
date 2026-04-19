#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fs_state.h"
#include "openF_helpers.h"

OpenFile openTable[MAX_OPEN] = {0};
int nextFD = 3;

static FileRecord fsEntries[FS_MAX_ENTRIES];
static int fsEntryCount = 0;
static int nextInode = 1;

static int ciCompare(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = (char)tolower((unsigned char)*a);
        char cb = (char)tolower((unsigned char)*b);

        if (ca != cb)
            return (ca < cb) ? -1 : 1;
        a++;
        b++;
    }

    if (*a == *b)
        return 0;
    return (*a == '\0') ? -1 : 1;
}

static void trimTrailingSlash(char *path)
{
    size_t len = strlen(path);

    while (len > 1 && path[len - 1] == '/') {
        path[len - 1] = '\0';
        len--;
    }
}

int fsNormalizePath(const char *input, char *outPath, size_t outSize)
{
    size_t i;
    size_t j = 0;
    int previousWasSlash = 0;

    if (input == NULL || outPath == NULL || outSize < 2)
        return 0;

    while (*input && isspace((unsigned char)*input))
        input++;

    if (*input == '\0')
        return 0;

    if (*input != '/' && *input != '\\') {
        if (j + 1 >= outSize)
            return 0;
        outPath[j++] = '/';
    }

    for (i = 0; input[i] != '\0'; i++) {
        char c = input[i];

        if (c == '\\')
            c = '/';

        if (isspace((unsigned char)c) && input[i + 1] == '\0')
            break;

        if (c == '/') {
            if (previousWasSlash)
                continue;
            previousWasSlash = 1;
        } else {
            previousWasSlash = 0;
        }

        if (j + 1 >= outSize)
            return 0;
        outPath[j++] = c;
    }

    outPath[j] = '\0';
    trimTrailingSlash(outPath);

    return outPath[0] == '/';
}

int fsExtractFileName(const char *path, char *outName, size_t outSize)
{
    char normalized[MAX_PATH];
    const char *lastSlash;

    if (!fsNormalizePath(path, normalized, sizeof(normalized)))
        return 0;

    lastSlash = strrchr(normalized, '/');
    if (lastSlash == NULL)
        lastSlash = normalized;
    else
        lastSlash++;

    if (*lastSlash == '\0')
        return 0;

    strncpy(outName, lastSlash, outSize - 1);
    outName[outSize - 1] = '\0';
    return 1;
}

int fsIsDirectoryPath(const char *path)
{
    const char *lastSlash;
    const char *lastDot;

    if (path == NULL || path[0] == '\0')
        return 0;

    lastSlash = strrchr(path, '/');
    lastDot = strrchr(path, '.');

    if (lastSlash == NULL)
        lastSlash = path - 1;

    return !(lastDot != NULL && lastDot > lastSlash + 1);
}

FileRecord *fsFindByPath(const char *path)
{
    char normalized[MAX_PATH];
    int i;

    if (!fsNormalizePath(path, normalized, sizeof(normalized)))
        return NULL;

    for (i = 0; i < fsEntryCount; i++) {
        if (strcmp(fsEntries[i].path, normalized) == 0)
            return &fsEntries[i];
    }

    return NULL;
}

FileRecord *fsFindByName(const char *name)
{
    int i;

    if (name == NULL || name[0] == '\0')
        return NULL;

    for (i = 0; i < fsEntryCount; i++) {
        if (ciCompare(fsEntries[i].name, name) == 0)
            return &fsEntries[i];
    }

    return NULL;
}

FileRecord *searchCentral(const char *name, TreeType *outType)
{
    unsigned char c;

    if (outType != NULL) {
        c = (unsigned char)(name != NULL ? name[0] : '\0');
        if (isalpha(c))
            *outType = TREE_ALPHA;
        else if (isdigit(c))
            *outType = TREE_NUM;
        else
            *outType = TREE_SYM;
    }

    return fsFindByName(name);
}

int fsCreateEntry(const char *path, int isDirectory)
{
    FileRecord *entry;
    char normalized[MAX_PATH];
    char name[MAX_NAME];

    if (fsEntryCount >= FS_MAX_ENTRIES)
        return 0;

    if (!fsNormalizePath(path, normalized, sizeof(normalized)))
        return 0;

    if (strcmp(normalized, "/") == 0)
        return 0;

    if (fsFindByPath(normalized) != NULL)
        return 0;

    if (!fsExtractFileName(normalized, name, sizeof(name)))
        return 0;

    entry = &fsEntries[fsEntryCount++];
    strncpy(entry->name, name, MAX_NAME - 1);
    entry->name[MAX_NAME - 1] = '\0';
    strncpy(entry->path, normalized, MAX_PATH - 1);
    entry->path[MAX_PATH - 1] = '\0';
    entry->content[0] = '\0';
    entry->inode = nextInode++;
    entry->size = 0;
    entry->isDirectory = isDirectory;
    return 1;
}

int insertFileCentral(const char *name)
{
    return fsCreateEntry(name, 0);
}

int fsGetEntries(FsEntryView entries[], int maxEntries)
{
    int i;
    int count;

    if (entries == NULL || maxEntries <= 0)
        return 0;

    count = (fsEntryCount < maxEntries) ? fsEntryCount : maxEntries;
    for (i = 0; i < count; i++) {
        strncpy(entries[i].name, fsEntries[i].name, MAX_NAME - 1);
        entries[i].name[MAX_NAME - 1] = '\0';
        strncpy(entries[i].path, fsEntries[i].path, MAX_PATH - 1);
        entries[i].path[MAX_PATH - 1] = '\0';
        entries[i].isDirectory = fsEntries[i].isDirectory;
        entries[i].size = fsEntries[i].size;
    }

    return count;
}

const char *fsGetPathByName(const char *name)
{
    FileRecord *rec = fsFindByName(name);
    return rec == NULL ? NULL : rec->path;
}

const char *fsGetContentByName(const char *name)
{
    FileRecord *rec = fsFindByName(name);
    return rec == NULL ? NULL : rec->content;
}

int fsSetContentByName(const char *name, const char *content)
{
    FileRecord *rec = fsFindByName(name);
    size_t len;

    if (rec == NULL || rec->isDirectory || content == NULL)
        return -1;

    strncpy(rec->content, content, MAX_CONTENT - 1);
    rec->content[MAX_CONTENT - 1] = '\0';
    len = strlen(rec->content);
    rec->size = (int)len;
    return rec->size;
}
