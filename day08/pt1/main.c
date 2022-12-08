#include <stdio.h>

#define toint(p) ((p)-48)

enum VDIR {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

int visit(int *array, int rows, int columns, int X, int Y, enum VDIR dir) {
    int isvisible = 0;
    int tree_height = 0;
    int cur_height = array[X + Y * columns];

    if (dir == DOWN) {
        /* DOWN */
        for (int y = Y + 1; y < rows; ++y) {
            tree_height = array[X + y * columns];
            if (tree_height < cur_height) {
                isvisible = 1;
            } else {
                isvisible = 0;
                break;
            }
        }
    } else if (dir == UP) {
        /* UP */
        for (int y = Y - 1; y >= 0; --y) {
            tree_height = array[X + y * columns];
            if (tree_height < cur_height) {
                isvisible = 1;
            } else {
                isvisible = 0;
                break;
            }
        }
    } else if (dir == LEFT) {
        /* LEFT */
        for (int x = X - 1; x >= 0; --x) {
            tree_height = array[x + Y * columns];
            if (tree_height < cur_height) {
                isvisible = 1;
            } else {
                isvisible = 0;
                break;
            }
        }
    } else {
        /* RIGHT */
        for (int x = X + 1; x < columns; ++x) {
            tree_height = array[x + Y * columns];
            if (tree_height < cur_height) {
                isvisible = 1;
            } else {
                isvisible = 0;
                break;
            }
        }
    }

    return isvisible;
}

int solve(int *array, int rows, int columns) {
    int acc = 0;
    /* Address the single array as though it was a matrix */
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            /* All outer perimeter trees can be seen */
            if ((y == 0 || y == rows - 1) || (x == 0 || x == columns - 1)) {
                acc++;
            } else {
                if (visit(array, rows, columns, x, y, UP) ||
                        visit(array, rows, columns, x, y, DOWN) ||
                        visit(array, rows, columns, x, y, LEFT) ||
                        visit(array, rows, columns, x, y, RIGHT))
                {
                    acc++;
                }
            }
        }
    }

    return acc;
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[BUFSIZ] = {0};
    int array[BUFSIZ * 100];
    int len = 0;
    int columns = 0;
    int rows = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        char *s = buf;
        columns = 0;
        while (*s != '\n') {
            array[len++] = toint(*s);
            s++;
            columns++;
        }
        rows++;
    }

    printf("%d\n", solve((int *)array, rows, columns));

    fclose(fp);
}
