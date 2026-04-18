#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "parser.h"

#define MAX_INPUT 100
#define HISTORY_SIZE 10

static struct termios original_tio;

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_tio);
}

void enableRawMode()
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, &original_tio);
    atexit(disableRawMode);

    raw = original_tio;
    raw.c_lflag &= ~(ICANON | ECHO);   /* character-by-character, no auto echo */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clearScreen()
{
    printf("\033[2J\033[H");   /* clear screen + move cursor home */
    fflush(stdout);
}

void redrawLine(const char *buffer)
{
    printf("\r\033[K{'e' to Exit} > %s", buffer);
    fflush(stdout);
}

void copyHistoryEntry(char *dest, const char *src)
{
    strncpy(dest, src, MAX_INPUT - 1);
    dest[MAX_INPUT - 1] = '\0';
}

void addToHistory(char history[HISTORY_SIZE][MAX_INPUT], int *count, const char *input)
{
    int i;

    if (input[0] == '\0')
        return;

    if (*count < HISTORY_SIZE) {
        copyHistoryEntry(history[*count], input);
        (*count)++;
    } else {
        for (i = 1; i < HISTORY_SIZE; i++)
            copyHistoryEntry(history[i - 1], history[i]);

        copyHistoryEntry(history[HISTORY_SIZE - 1], input);
    }
}

void readInputWithHistory(char *buffer, char history[HISTORY_SIZE][MAX_INPUT], int historyCount)
{
    int len = 0;
    int historyIndex = historyCount;   /* one past newest = blank line */
    char c;

    buffer[0] = '\0';

    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1)
            continue;

        if (c == '\n' || c == '\r') {
            buffer[len] = '\0';
            printf("\n");
            return;
        }

        /* backspace */
        if (c == 127 || c == 8) {
            if (len > 0) {
                len--;
                buffer[len] = '\0';
                redrawLine(buffer);
            }
            continue;
        }

        /* escape sequence for arrows */
        if (c == 27) {
            char seq[2];

            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;

            if (seq[0] == '[') {
                /* up arrow */
                if (seq[1] == 'A') {
                    if (historyCount > 0 && historyIndex > 0) {
                        historyIndex--;
                        copyHistoryEntry(buffer, history[historyIndex]);
                        len = (int)strlen(buffer);
                        redrawLine(buffer);
                    }
                }
                /* down arrow */
                else if (seq[1] == 'B') {
                    if (historyIndex < historyCount - 1) {
                        historyIndex++;
                        copyHistoryEntry(buffer, history[historyIndex]);
                        len = (int)strlen(buffer);
                        redrawLine(buffer);
                    } else if (historyIndex == historyCount - 1) {
                        historyIndex = historyCount;
                        buffer[0] = '\0';
                        len = 0;
                        redrawLine(buffer);
                    }
                }
            }
            continue;
        }

        /* Ctrl+L clears screen like Linux terminal */
        if (c == 12) {
            clearScreen();
            redrawLine(buffer);
            continue;
        }

        if (len < MAX_INPUT - 1) {
            buffer[len++] = c;
            buffer[len] = '\0';
            write(STDOUT_FILENO, &c, 1);
        }
    }
}

int main()
{
    char input[MAX_INPUT];
    char history[HISTORY_SIZE][MAX_INPUT] = {{0}};
    int historyCount = 0;

    enableRawMode();
    clearScreen();

    while (1) {
        printf("{'e' to Exit} > ");
        fflush(stdout);

        readInputWithHistory(input, history, historyCount);

        if (strcmp(input, "e") == 0) {
            printf("Exiting...\n");
            break;
        }

        if (strcmp(input, "clear") == 0) {
            clearScreen();
            addToHistory(history, &historyCount, input);
            continue;
        }

        addToHistory(history, &historyCount, input);
        parseInstruct(input);
    }

    return 0;
}
