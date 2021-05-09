/* TermUtil *nix terminal utility library 1.4 (C) 2021 PQCraft */

#ifndef TERMUTIL_H
#define TERMUTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

struct termios term, restore;
static struct termios orig_termios;

bool textlock = false;

int curx, cury;

struct cursor {
    int x;
    int y;
};

struct cursor* srstack = NULL;
int srstackp = 0;

// Clears the terminal
void clear() {
    fputs("\e[H\e[2J\e[3J", stdout);
    fflush(stdout);
}

// Clears the terminal without moving the cursor
void blank() {
    fputs("\e[2J\e[3J", stdout);
    fflush(stdout);
}

// Gets the size of the terminal and returns a winsize struct
// Use the ws_col and ws_row winsize struct variables to get the size
struct winsize getTermSize() {
    struct winsize tmp;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &tmp);
    return tmp;
}


// Gets the number of characters in stdin
int kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;
    if (!initialized) {
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }
    int bytes;
    ioctl(STDIN, FIONREAD, &bytes);
    return bytes;
}

// Returns the cursor position as a cursor struct
struct cursor getCurPos() {
    fflush(stdout);
    char buf[16];
    register int ret, i;
    i = kbhit();
    while (i > 0) {getchar(); i--;}
    if (!textlock) {
        tcgetattr(0, &term);
        tcgetattr(0, &restore);
        term.c_lflag &= ~(ICANON|ECHO);
        tcsetattr(0, TCSANOW, &term);
    }
    fputs("\e[6n", stdout);
    fflush(stdout);
    ret = read(1, &buf, 16);
	struct cursor cur = {0, 0};
    if (!ret) {
        if (!textlock) tcsetattr(0, TCSANOW, &restore);
        return cur;
    }
	if (!textlock) tcsetattr(0, TCSANOW, &restore);
    sscanf(buf, "\e[%d;%dR", &cur.y, &cur.x);
    return cur;
}

// Moves the cursor position using a cursor struct
void setCurPos(struct cursor cur) {
    printf("\e[%d;%dH", cur.y, cur.x);
    fflush(stdout);
}

// Pushes the current cursor position onto a stack
void saveCurPos() {
    srstackp++;
    srstack = (struct cursor*)realloc(srstack, srstackp * sizeof(srstack));
    srstack[srstackp - 1] = getCurPos();
}

// Pops the current cursor position
void restoreCurPos() {
    if (srstackp < 0) return;
    srstackp--;
    setCurPos(srstack[srstackp]);
    srstack = (struct cursor*)realloc(srstack, srstackp * sizeof(srstack));
}

// Moves the cursor to (x, y) and, places c, and moves back
void putCharAt(int x, int y, char c) {
    printf("\e[s\e[%d;%dH%c\e[u", y, x, c);
    fflush(stdout);
}

// Moves the cursor to (x, y) and, writes s, and moves back
void putStrAt(int x, int y, char* s) {
    printf("\e[s\e[%d;%dH%s\e[u", y, x, s);
    fflush(stdout);
}

// Reset the text effects
#define reset() fputs("\e[0m", stdout)

// Set the text foreground color to a 24-bit color value
#define trueColorFG(color) printf("\e[38;2;%u;%u;%um", (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color)

// Set the text background color to a 24-bit color value
#define trueColorBG(color) printf("\e[48;2;%u;%u;%um", (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color)

// Set the text foreground color to a 8-bit/1-byte color value
// Called byteColor because 8bitColor is an invalid name
#define byteColorFG(color) printf("\e[38;5;%um", (uint8_t)color)

// Set the text background color to a 8-bit/1-byte color value
#define byteColorBG(color) printf("\e[48;5;%um", (uint8_t)color)

// Make the text bold
#define bold() fputs("\e[1m", stdout)

// Make the text bold
#define italic() fputs("\e[3m", stdout)

// Make the text bold
#define underline() fputs("\e[4m", stdout)

// Make the text bold
#define doubleUnderline() fputs("\e[21m", stdout)

// Make the text bold
#define squiggleUnderline() fputs("\e[4:3m", stdout)

// Make the text bold
#define strikethrough() fputs("\e[9m", stdout)

// Make the text bold
#define overline() fputs("\e[53m", stdout)

// Make the text bold
#define dim() fputs("\e[2m", stdout)

// Make the text bold
#define blink() fputs("\e[5m", stdout)

// Make the text bold
#define hide() fputs("\e[8m", stdout)

// Make the text bold
#define reverse() fputs("\e[7m", stdout)

// Stops the terminal from printing characters when the user presses a key
void lockTerm() {
    if (!textlock) {
        tcgetattr(0, &term);
        tcgetattr(0, &restore);
        term.c_lflag &= ~(ICANON|ECHO);
        tcsetattr(0, TCSANOW, &term);
        textlock = true;
    }
}

// Re-enables terminal echoing
void unlockTerm() {
    if (textlock) {
        tcsetattr(0, TCSANOW, &restore);
        textlock = false;
    }
}

// Detects it you are in a terminal
#define tty() (bool)!system("tty -s 1> /dev/null 2> /dev/null")

#ifdef TERMUTIL_EXTRAS
// Returns a random double
double randDouble(double min, double max) {
    double range = max - min;
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

// Converts a string to upper-case
void upCase(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 32;
    }
}

// Converts a string to lower-case
void lowCase(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') str[i] += 32;
    }
}

// Concatenates a single char to the end of a string
char* strcatchar(char* str, char c) {
    int len = 0;
    while (str[len]) {len++;}
    str[len] = c;
    len++;
    str[len] = 0;
    return str;
}

// Copies part of a string to another string
char* strcpysnip(char* str2, char* str1, int i, int j) {
    int i2 = 0;
    for (int i3 = i; i3 < j; i3++) {str2[i2] = str1[i3]; i2++;}
    str2[i2] = 0;
    return str2;
}

// Concatenates part of a string to the end of another string
char* strcatsnip(char* str3, char* str1, int i, int j) {
    char* str2 = malloc(j - i + 1);
    int i2 = 0;
    for (int i3 = i; i3 < j; i3++) {str2[i2] = str1[i3]; i2++;}
    str2[i2] = 0;
    strcat(str3, str2);
    free(str2);
    return str3;
}
#endif

#endif