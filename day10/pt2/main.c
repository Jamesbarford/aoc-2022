#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRT_ROWS (6)
#define CRT_COLUMNS (40)

static char crt_screen[CRT_ROWS][CRT_COLUMNS];

void printGrid(void) {
    for (int y = 0; y < CRT_ROWS; ++y) {
        printf("[%d]", y);
        for (int x = 0; x < CRT_COLUMNS; ++x) {
            printf("%c", crt_screen[y][x]);
        }
        printf("\n");
    }
    printf("\n");
}

void plotOnGrid(int x, int y, char c) {
    crt_screen[y][x] = c;
}

char getPixel(int x, int Xreg) {
    if (x - 1 == Xreg || x == Xreg || x + 1 == Xreg) {
        return '#';
    }
    return '.';
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[64] = {0};
    int Xreg = 1;
    int cycle = 0;
    int y = 0;
    int x = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        int isaddx = strncasecmp(buf, "addx", 4) == 0;
        int iterations = isaddx == 1 ? 2 : 1;

        while (iterations-- && ++cycle) {
            x = (cycle - 1) % 40;
            plotOnGrid(x, y, getPixel(x, Xreg));
            if ((cycle % 40) == 0) {
                ++y;
            }
        }

        if (isaddx) {
            Xreg += (int)strtol(buf + 5, NULL, 10);
        }
    }

    fclose(fp);
    printGrid();
    return 0;
}
