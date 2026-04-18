#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fs_state.h"
#include "openF_helpers.h"

static void trimTrailingNewline(char *s)
{
    size_t len;

    if (s == NULL)
        return;

    len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

static void printEditorHelp(void)
{
    printf("Editor commands:\n");
    printf("  :p                 print current buffer with line numbers\n");
    printf("  :i                 insert mode at end of file\n");
    printf("  :i <line>          insert mode before line number\n");
    printf("  :a                 append mode after end of file\n");
    printf("  :a <line>          append mode after line number\n");
    printf("  :d <line>          delete a line\n");
    printf("  :r <line>          replace a line\n");
    printf("  :w                 write buffer to file\n");
    printf("  :wq                write and quit\n");
    printf("  :q                 quit without saving pending changes\n");
    printf("  :help              show this command list\n");
    printf("  .                  in insert/append mode, finish text entry\n");
}

static int loadBufferIntoLines(const char *content,
                               char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH],
                               int *lineCount)
{
    char temp[MAX_EDIT_BUFFER];
    char *token;

    *lineCount = 0;

    if (content == NULL || content[0] == '\0')
        return 1;

    strncpy(temp, content, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    token = strtok(temp, "\n");
    while (token != NULL) {
        if (*lineCount >= MAX_EDIT_LINES) {
            printf("openF: editor buffer exceeded maximum line count\n");
            return 0;
        }

        strncpy(lines[*lineCount], token, MAX_LINE_LENGTH - 1);
        lines[*lineCount][MAX_LINE_LENGTH - 1] = '\0';
        (*lineCount)++;
        token = strtok(NULL, "\n");
    }

    return 1;
}

static void printLines(char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH], int lineCount)
{
    int i;

    if (lineCount == 0) {
        printf("[empty file]\n");
        return;
    }

    for (i = 0; i < lineCount; i++)
        printf("%4d  %s\n", i + 1, lines[i]);
}

static int insertLineAt(char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH],
                        int *lineCount, int index, const char *text)
{
    int i;

    if (*lineCount >= MAX_EDIT_LINES) {
        printf("openF: maximum line capacity reached\n");
        return 0;
    }

    if (index < 0)
        index = 0;
    if (index > *lineCount)
        index = *lineCount;

    for (i = *lineCount; i > index; i--)
        strcpy(lines[i], lines[i - 1]);

    strncpy(lines[index], text, MAX_LINE_LENGTH - 1);
    lines[index][MAX_LINE_LENGTH - 1] = '\0';
    (*lineCount)++;
    return 1;
}

static int collectInsertText(char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH],
                             int *lineCount, int index)
{
    char line[MAX_LINE_LENGTH];

    printf("-- INSERT -- (enter . alone on a line to stop)\n");
    while (1) {
        printf("| ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\nInsert ended.\n");
            return 1;
        }

        trimTrailingNewline(line);
        if (strcmp(line, ".") == 0)
            break;

        if (!insertLineAt(lines, lineCount, index, line))
            return 0;
        index++;
    }

    return 1;
}

static void rebuildBufferFromLines(char *buffer, size_t bufferSize,
                                   char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH],
                                   int lineCount)
{
    int i;
    size_t used = 0;

    if (bufferSize == 0)
        return;

    buffer[0] = '\0';

    for (i = 0; i < lineCount; i++) {
        size_t len = strlen(lines[i]);
        size_t need = len + 1;

        if (used + need >= bufferSize)
            break;

        memcpy(buffer + used, lines[i], len);
        used += len;
        buffer[used++] = '\n';
        buffer[used] = '\0';
    }
}

static int saveEditorBuffer(FileRecord *file,
                            char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH],
                            int lineCount)
{
    char editBuffer[MAX_EDIT_BUFFER];
    int newSize;

    rebuildBufferFromLines(editBuffer, sizeof(editBuffer), lines, lineCount);
    newSize = fsSetContentByName(file->name, editBuffer);
    if (newSize >= 0)
        file->size = newSize;

    printf("Saved '%s' (%d bytes).\n", file->name, file->size);
    return 1;
}

int fileHasActiveReadableHandle(const char *name)
{
    int i;
    int found = 0;

    for (i = 0; i < MAX_OPEN; i++) {
        if (!openTable[i].used || strcmp(openTable[i].name, name) != 0)
            continue;

        found = 1;
        if (strcmp(openTable[i].mode, "r") == 0 || strcmp(openTable[i].mode, "rw") == 0)
            return 1;
    }

    return found ? 0 : 1;
}

int fileHasActiveWritableHandle(const char *name)
{
    int i;
    int found = 0;

    for (i = 0; i < MAX_OPEN; i++) {
        if (!openTable[i].used || strcmp(openTable[i].name, name) != 0)
            continue;

        found = 1;
        if (strcmp(openTable[i].mode, "w") == 0 || strcmp(openTable[i].mode, "rw") == 0)
            return 1;
    }

    return found ? 0 : 1;
}

int fileCurrentlyOpen(const char *name)
{
    int i;

    for (i = 0; i < MAX_OPEN; i++) {
        if (openTable[i].used && strcmp(openTable[i].name, name) == 0)
            return 1;
    }

    return 0;
}

void listClosedFiles(void)
{
    FsEntryView entries[FS_MAX_ENTRIES];
    int count;
    int i;
    int found = 0;

    count = fsGetEntries(entries, FS_MAX_ENTRIES);

    printf("Closed files available to open:\n");
    for (i = 0; i < count; i++) {
        if (entries[i].isDirectory)
            continue;
        if (fileCurrentlyOpen(entries[i].name))
            continue;

        printf("  %s\n", entries[i].path);
        found = 1;
    }

    if (!found)
        printf("  (none)\n");
}

void showFileContent(FileRecord *file)
{
    const char *path;
    const char *content;

    if (!fileHasActiveReadableHandle(file->name)) {
        printf("openF: cannot display '%s': file is currently open without read permission\n", file->name);
        return;
    }

    path = fsGetPathByName(file->name);
    content = fsGetContentByName(file->name);

    printf("----- %s -----\n", path == NULL ? file->name : path);
    if (content == NULL || content[0] == '\0')
        printf("[empty file]\n");
    else
        printf("%s", content);

    if (content != NULL && content[0] != '\0' && content[strlen(content) - 1] != '\n')
        printf("\n");
    printf("----------------\n");
}

void editFile(FileRecord *file)
{
    char command[MAX_LINE_LENGTH];
    char lines[MAX_EDIT_LINES][MAX_LINE_LENGTH];
    const char *existing;
    int lineCount = 0;
    int modified = 0;
    int target;

    if (!fileHasActiveWritableHandle(file->name)) {
        printf("openF: cannot edit '%s': file is currently open without write permission\n", file->name);
        return;
    }

    disableRawMode();

    existing = fsGetContentByName(file->name);
    if (!loadBufferIntoLines(existing, lines, &lineCount)) {
        enableRawMode();
        return;
    }

    printf("Entering vi-style edit mode for '%s'\n", file->name);
    printf("Type :help for commands. Current buffer loaded into memory.\n");
    printLines(lines, lineCount);

    while (1) {
        printf(": ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\nEdit cancelled.\n");
            break;
        }

        trimTrailingNewline(command);

        if (strcmp(command, ":help") == 0) {
            printEditorHelp();
        } else if (strcmp(command, ":p") == 0) {
            printLines(lines, lineCount);
        } else if (strcmp(command, ":i") == 0) {
            if (collectInsertText(lines, &lineCount, lineCount))
                modified = 1;
        } else if (sscanf(command, ":i %d", &target) == 1) {
            if (target < 1 || target > lineCount + 1) {
                printf("Invalid line number. Valid range: 1 to %d\n", lineCount + 1);
            } else if (collectInsertText(lines, &lineCount, target - 1)) {
                modified = 1;
            }
        } else if (strcmp(command, ":a") == 0) {
            if (collectInsertText(lines, &lineCount, lineCount))
                modified = 1;
        } else if (sscanf(command, ":a %d", &target) == 1) {
            if (target < 0 || target > lineCount) {
                printf("Invalid line number. Valid range: 0 to %d\n", lineCount);
            } else if (collectInsertText(lines, &lineCount, target)) {
                modified = 1;
            }
        } else if (sscanf(command, ":d %d", &target) == 1) {
            int i;

            if (target < 1 || target > lineCount) {
                printf("Invalid line number. Valid range: 1 to %d\n", lineCount);
                continue;
            }

            for (i = target - 1; i < lineCount - 1; i++)
                strcpy(lines[i], lines[i + 1]);
            lineCount--;
            modified = 1;
            printf("Deleted line %d.\n", target);
        } else if (sscanf(command, ":r %d", &target) == 1) {
            char replacement[MAX_LINE_LENGTH];

            if (target < 1 || target > lineCount) {
                printf("Invalid line number. Valid range: 1 to %d\n", lineCount);
                continue;
            }

            printf("replace line %d> ", target);
            fflush(stdout);
            if (fgets(replacement, sizeof(replacement), stdin) == NULL) {
                printf("Replacement cancelled.\n");
                continue;
            }
            trimTrailingNewline(replacement);
            strncpy(lines[target - 1], replacement, MAX_LINE_LENGTH - 1);
            lines[target - 1][MAX_LINE_LENGTH - 1] = '\0';
            modified = 1;
        } else if (strcmp(command, ":w") == 0) {
            saveEditorBuffer(file, lines, lineCount);
            modified = 0;
        } else if (strcmp(command, ":wq") == 0) {
            saveEditorBuffer(file, lines, lineCount);
            modified = 0;
            break;
        } else if (strcmp(command, ":q") == 0) {
            if (modified) {
                printf("Unsaved changes. Use :wq to save and quit, or enter :q again to discard.\n");
                modified = -1;
            } else {
                printf("Exited editor without saving.\n");
                break;
            }
        } else if (modified == -1 && strcmp(command, ":q") == 0) {
            printf("Discarded changes.\n");
            break;
        } else if (command[0] == '\0') {
            continue;
        } else {
            printf("Unknown editor command. Type :help for options.\n");
            if (modified == -1)
                modified = 1;
        }

        if (modified == -1)
            continue;
    }

    enableRawMode();
}
