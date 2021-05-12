/* TermUtil *nix terminal utility library 1.5 (C) 2021 PQCraft */

/* NOTE: Yes, I am aware that defining functions in headers is */
/* not standard procedure, but I want this library to be easy  */
/* to include and not spread across multiple files.            */

#ifndef TERMUTIL_H
#define TERMUTIL_H

// Required libraries
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

// Variables
struct termios tu_term, tu_old_term;
bool tu_textlock = false;
struct tu_cursor_struct {
    int x;
    int y;
};
#define tu_cursor_t struct tu_cursor_struct
tu_cursor_t* srstack = NULL;
int srstackp = 0;

// Clears the terminal
void tu_clear() {
    fputs("\e[H\e[2J\e[3J", stdout);
    fflush(stdout);
}

// Clears the terminal without moving the cursor
void tu_blank() {
    fputs("\e[2J\e[3J", stdout);
    fflush(stdout);
}

// Gets the size of the terminal
// Returns: winsize struct containing the height and width of the terminal
// - Use the 'ws_col' and 'ws_row' winsize struct members to get the size
struct winsize tu_getTermSize() {
    struct winsize tmp;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &tmp);
    return tmp;
}


// Gets the number of characters in stdin (mimics the Windows function)
// Returns: Number of characters in stdin
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

// Returns the cursor position as a tu_cursor_struct
// Returns: A tu_cursor_t containing the cursor position
tu_cursor_t tu_getCurPos() {
    fflush(stdout);
    char buf[16];
    register int ret, i;
    i = kbhit();
    while (i > 0) {getchar(); i--;}
    if (!tu_textlock) {
        tcgetattr(0, &tu_term);
        tcgetattr(0, &tu_old_term);
        tu_term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &tu_term);
    }
    fputs("\e[6n", stdout);
    fflush(stdout);
    ret = read(1, &buf, 16);
	tu_cursor_t cur = {0, 0};
    if (!ret) {
        if (!tu_textlock) tcsetattr(0, TCSANOW, &tu_old_term);
        return cur;
    }
	if (!tu_textlock) tcsetattr(0, TCSANOW, &tu_old_term);
    sscanf(buf, "\e[%d;%dR", &cur.y, &cur.x);
    return cur;
}

// Moves the cursor position using a tu_cursor struct
// cur = A tu_cursor_t containing the new cursor's x and y
void tu_setCurPos(tu_cursor_t cur) {
    printf("\e[%d;%dH", cur.y, cur.x);
    fflush(stdout);
}

// Pushes the current cursor position onto a stack
void tu_saveCurPos() {
    srstackp++;
    srstack = (tu_cursor_t*)realloc(srstack, srstackp * sizeof(srstack));
    srstack[srstackp - 1] = tu_getCurPos();
}

// Pops the current cursor position
void tu_restoreCurPos() {
    if (srstackp < 0) return;
    srstackp--;
    tu_setCurPos(srstack[srstackp]);
    srstack = (tu_cursor_t*)realloc(srstack, srstackp * sizeof(srstack));
}

// Moves the cursor to (x, y) and, places c, and moves back
// x = X position of character
// y = Y position of character
// c = Character to place
void tu_putCharAt(int x, int y, char c) {
    printf("\e[s\e[%d;%dH%c\e[u", y, x, c);
    fflush(stdout);
}

// Moves the cursor to (x, y) and, writes s, and moves back
// x = X position of string
// y = Y position of string
// c = Character to string
void tu_putStrAt(int x, int y, char* s) {
    printf("\e[s\e[%d;%dH%s\e[u", y, x, s);
    fflush(stdout);
}

// Reset the text effects
#define tu_reset() fputs("\e[0m", stdout)

// Set the text foreground color to a 24-bit color value
#define tu_trueColorFG(color) printf("\e[38;2;%u;%u;%um", (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color)

// Set the text background color to a 24-bit color value
#define tu_trueColorBG(color) printf("\e[48;2;%u;%u;%um", (uint8_t)(color >> 16), (uint8_t)(color >> 8), (uint8_t)color)

// Set the text foreground color to a 8-bit/1-byte color value
// Called byteColor* because 256Color 8bitColor are invalid names
#define tu_byteColorFG(color) printf("\e[38;5;%um", (uint8_t)color)

// Set the text background color to a 8-bit/1-byte color value
// Called byteColor* because 256Color 8bitColor are invalid names
#define tu_byteColorBG(color) printf("\e[48;5;%um", (uint8_t)color)

// Turn on bold effect
#define tu_bold() fputs("\e[1m", stdout)

// Turn on italic effect
#define tu_italic() fputs("\e[3m", stdout)

// Turn on underline effect
#define tu_underline() fputs("\e[4m", stdout)

// Turn on double underline effect
#define tu_doubleUnderline() fputs("\e[21m", stdout)

// Turn on squiggly underline effect
#define tu_squiggleUnderline() fputs("\e[4:3m", stdout)

// Turn on strike-through effect
#define tu_strikethrough() fputs("\e[9m", stdout)

// Turn on over-line effect
#define tu_overline() fputs("\e[53m", stdout)

// Turn on dim effect
#define tu_dim() fputs("\e[2m", stdout)

// Turn on blink effect
#define tu_blink() fputs("\e[5m", stdout)

// Enable text hiding
#define tu_hide() fputs("\e[8m", stdout)

// Reverse text colors
#define tu_reverse() fputs("\e[7m", stdout)

// Stops the terminal from printing characters when the user presses a key
void tu_lockTerm() {
    if (!tu_textlock) {
        tcgetattr(0, &tu_term);
        tcgetattr(0, &tu_old_term);
        tu_term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &tu_term);
        tu_textlock = true;
    }
}

// Re-enables terminal echoing
void tu_unlockTerm() {
    if (tu_textlock) {
        tcsetattr(0, TCSANOW, &tu_old_term);
        tu_textlock = false;
    }
}

// Detects it you are in a terminal
#define intty() !system("tty -s 1> /dev/null 2> /dev/null")

// Define 'TERMUTIL_EXTRAS' to include functions not related to the terminal
#ifdef TERMUTIL_EXTRAS

// Returns a random double
// min = Minimum number
// max = Maximum number
// Returns: random double in the range of min to max
double randDouble(double min, double max) {
    double range = max - min;
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

// Converts a string to upper-case
// str = Pointer to string
void upCase(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 32;
    }
}

// Converts a string to lower-case
// str = Pointer to string
void lowCase(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') str[i] += 32;
    }
}

// Concatenates a single char to the end of a string
// str = String to append char to
// Returns: str (standard behavior of strcat)
char* strcatchar(char* str, char c) {
    int len = 0;
    while (str[len]) {len++;}
    str[len] = c;
    len++;
    str[len] = 0;
    return str;
}

// Copies part of a string to another string
// str2 = String to copy to
// str1 = String to read snippet from
// i = starting position
// j = ending position
// Returns: str (standard behavior of strcpy)
char* strcpysnip(char* str2, char* str1, int i, int j) {
    int i2 = 0;
    for (int i3 = i; i3 < j; i3++) {str2[i2] = str1[i3]; i2++;}
    str2[i2] = 0;
    return str2;
}

// Concatenates part of a string to the end of another string
// str2 = String to copy to
// str1 = String to read snippet from
// i = starting position
// j = ending position
// Returns: str (standard behavior of strcat)
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