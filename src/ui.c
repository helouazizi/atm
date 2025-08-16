// src/ui.c
#include <sys/ioctl.h>
#include <unistd.h>
#include "header.h"


int getTerminalWidth()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}


// Print centered text
void printCentered(const char *text) {
    int width = getTerminalWidth();
    int len = (int)strlen(text);
    int padding = (width - len) / 2;
    if (padding < 0) padding = 0;
    printf("%*s%s\n", padding + len, text, RESET);
}


// Print separator line
void printSeparator(char ch) {
    int width = getTerminalWidth();
    for (int i = 0; i < width; i++)
        putchar(ch);
    putchar('\n');
}