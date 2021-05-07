/* TermUtil *nix terminal utility library 1.1 (C) 2021 PQCraft */

#ifndef TERMUTIL_H
#define TERMUTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

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
    fflush(stdout);
    if (srstackp) srstack = (struct cursor*)realloc(srstack, srstackp * sizeof(srstack));
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
bool tty() {
    return (bool)!system("tty -s 1> /dev/null 2> /dev/null");
}

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
void strcatchar(char* str, char c) {
    int len = 0;
    while (str[len]) {len++;}
    str[len] = c;
    len++;
    str[len] = 0;
}

// Copies part of a string to another string
void strcpysnip(char* str2, char* str1, int i, int j) {
    if (!str1) {str2[0] = 0; return;}
    int i2 = 0;
    for (int i3 = i; i3 < j; i3++) {str2[i2] = str1[i3]; i2++;}
    str2[i2] = 0;
}

// Concatenates part of a string to the end of another string
void strcpysnip(char* str3, char* str1, int i, int j) {
    char* str2 = malloc(j - i + 1);
    if (!str1) {str2[0] = 0; return;}
    int i2 = 0;
    for (int i3 = i; i3 < j; i3++) {str2[i2] = str1[i3]; i2++;}
    str2[i2] = 0;
    strcat(str3, str2);
    free(str2);
}

#endif