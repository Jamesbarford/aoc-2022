#include <stdio.h>

#define toint(p) ((p)-48)
#define MAX_ARRAY_SIZE (10000)

enum VDIR {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

int visit(int *array, int rows, int columns, int X, int Y, enum VDIR dir) {
    int acc = 0;
    int tree_height = 0;
    int cur_height = array[X + Y * columns];

    if (dir == UP) {
        /* UP  */
        for (int y = Y - 1; y >= 0; --y) {
            tree_height = array[X + y * columns];
            acc++;
            if (tree_height >= cur_height) {
                break;
            }
        }
    } else if (dir == DOWN) {
        /* DOWN */
        for (int y = Y + 1; y < rows; ++y) {
            tree_height = array[X + y * columns];
            acc++;
            if (tree_height >= cur_height) {
                break;
            }
        }
    } else if (dir == LEFT) {
        /* LEFT */
        for (int x = X - 1; x >= 0; --x) {
            tree_height = array[x + Y * columns];
            acc++;
            if (tree_height >= cur_height) {
                break;
            }
        }
    } else {
        /* RIGHT */
        for (int x = X + 1; x < columns; ++x) {
            tree_height = array[x + Y * columns];
            acc++;
            if (tree_height >= cur_height) {
                break;
            }
        }
    }

    return acc;
}

int solve(int *array, int rows, int columns) {
    int heap[MAX_ARRAY_SIZE] = {0};
    int heapsize = 0;

    /* Address the single array as though it was a matrix */
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            if ((y == 0 || y == rows - 1) || (x == 0 || x == columns - 1)) {
                continue;
            } else {
                int up = visit(array, rows, columns, x, y, UP);
                int down = visit(array, rows, columns, x, y, DOWN);
                int left = visit(array, rows, columns, x, y, LEFT);
                int right = visit(array, rows, columns, x, y, RIGHT);
                int score = (up * down * left * right);

                /* Insert into the heap O(log N) */
                heap[++heapsize] = score;
                int cur = heapsize;
                while (cur > 0 && heap[cur / 2] < heap[cur]) {
                    int parent = cur / 2;
                    int tmp = heap[parent];
                    heap[parent] = heap[cur];
                    heap[cur] = tmp;
                    cur = parent;
                }
            }
        }
    }

    /* Heap is sorted by max, so return first item */
    return heap[0];
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[MAX_ARRAY_SIZE] = {0};
    int array[MAX_ARRAY_SIZE];
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
    fclose(fp);

    printf("%d\n", solve((int *)array, rows, columns));
}
