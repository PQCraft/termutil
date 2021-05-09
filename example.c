#include "termutil.h"

int main() {
    if (!tty()) return 0;
    clear();
    printf("John Cena");
    blank();
    printf(">>>");
    putCharAt(3, 3, 'E');
    setCurPos((struct cursor){3, 4});
    printf("Test");
    saveCurPos();
    printf("\nbruh");
    restoreCurPos();
    puts(" text.\n");
    puts("Default");
    bold();
    puts("+bold");
    italic();
    puts("+italic");
    trueColorFG(0xFF0000);
    puts("RED");
    trueColorFG(0x00FF00);
    puts("GREEN");
    trueColorFG(0x0000FF);
    puts("BLUE");
    reset();
    puts("Default");
    return 0;
}
