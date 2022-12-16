#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_MAX_CAP (170)

#define X_OFFSET 280     // 480
#define WORLD_WIDTH 500  // 50
#define WORLD_HEIGHT 300 // 15

typedef struct lNode {
    int start, end;
    struct lNode *next;
} lNode;

/* FIFO QUEUE */
typedef struct list {
    int size;
    lNode *root;
    lNode *tail;
} list;

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->root = l->tail = NULL;
    l->size = 0;
    return l;
}

void listAppend(list *l, int x, int y) {
    lNode *ln = malloc(sizeof(lNode));
    ln->start = x;
    ln->end = y;
    ln->next = NULL;

    if (l->root == NULL) {
        l->root = l->tail = ln;
    } else {
        l->tail->next = ln;
        l->tail = ln;
    }

    l->size++;
}

lNode *listDequeue(list *l) {
    if (l->size == 0) {
        return NULL;
    }

    lNode *rem = l->root;
    l->root = rem->next;
    l->size--;
    rem->next = NULL;
    return rem;
}

void listRelease(list *l) {
    if (l) {
        lNode *ln = l->root;
        lNode *next = NULL;
        while (ln) {
            next = ln->next;
            free(ln);
            ln = next;
        }
        free(l);
    }
}

void listPrint(list *l) {
    int len = l->size;
    lNode *ln = l->root;
    printf("[");
    while (ln && len) {
        printf("(%d, %d)", ln->start, ln->end);
        if (len - 1 != 0) {
            printf(", ");
        }

        --len;
        ln = ln->next;
    }
    printf("]\n");
}

typedef struct grid {
    int rows;
    int columns;
    char *buf;
} grid;

grid *gridNew(int bufsize) {
    grid *g = malloc(sizeof(grid));
    g->rows = 0;
    g->columns = 0;
    g->buf = calloc(bufsize, sizeof(char));
    return g;
}

char gridGet(grid *g, int x, int y) {
    int idx = x + y * g->columns;
    if (x < 0 || x >= g->columns) {
        return -1;
    } else if (y < 0 || y >= g->rows) {
        return -1;
    }
    return g->buf[idx];
}

int gridFind(grid *g, char c, int *dx, int *dy) {
    for (int y = 0; y < g->rows; ++y) {
        for (int x = 0; x < g->columns; ++x) {
            if (g->buf[x + y * g->columns] == c) {
                *dx = x;
                *dy = y;
                return 1;
            }
        }
    }
    return 0;
}

void swap(int *i, int *j) {
    int tmp = *i;
    *i = *j;
    *j = tmp;
}

void plotLine(grid *g, int x0, int y0, int x1, int y1, char c) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int err2;

    while (1) {
        g->buf[x0 + y0 * g->columns] = c;
        if (x0 == x1 && y0 == y1)
            break;
        err2 = err * 2;
        if (err2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (err2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void gridPrint(grid *g) {
    printf("columns: %d\nrows: %d\n", g->columns, g->rows);
    for (int y = 0; y < g->rows; ++y) {
        for (int x = 0; x < g->columns; ++x) {
            printf("%c", g->buf[x + y * g->columns] == '\0'
                                 ? '.'
                                 : g->buf[x + y * g->columns]);
        }
        printf("\n");
    }
}

void gridRelease(grid *g) {
    if (g) {
        free(g->buf);
        free(g);
    }
}

int gridPlot(grid *g, list **lists, int list_count) {
    for (int i = 0; i < list_count; ++i) {
        list *l1 = lists[i];
        lNode *cur = l1->root->next;
        lNode *prev = l1->root;

        while (cur && prev) {
            plotLine(g, prev->start, prev->end, cur->start, cur->end, '#');
            prev = cur;
            cur = cur->next;
        }
    }

    return 1;
}

int dir[][2] = {
        {0, 1},
        {-1, 1},
        {1, 1},
};

void simulateSand(grid *g, int rest[2]) {
    int x = 500 - X_OFFSET;
    int y = 0;

    while (1) {
        int i;
        for (i = 0; i < 3; ++i) {
            int x1 = x + dir[i][0];
            int y1 = y + dir[i][1];
            int plot = gridGet(g, x1, y1);
            if (plot == -1) {
                rest[0] = rest[1] = -1;
                return;
            }
            if (plot != 0) {
                continue;
            }
            x = x1;
            y = y1;
            break;
        }

        if (i == 3) {
            rest[0] = x;
            rest[1] = y;
            g->buf[x + y * g->columns] = 'o';
            return;
        }
    }
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char tmp[BUFSIZ];
    int list_count = 0;
    int maxX, maxY, minX, minY;
    list *lists[LIST_MAX_CAP];
    grid *g = gridNew(WORLD_WIDTH * WORLD_HEIGHT);

    maxX = maxY = INT_MIN;
    minX = minY = INT_MAX;

    while (fgets(tmp, sizeof(tmp), fp)) {
        char *ptr = tmp;
        size_t len = strlen(ptr);
        lists[list_count] = listNew();
        list *l = lists[list_count];
        list_count++;
        if (ptr[len - 1] == '\n') {
            ptr[len - 1] = '\0';
            len--;
        }
        int x = 0, y = 0;
        int parse_x = 1;

        while (*ptr) {
            switch (*ptr) {
                case '-':
                    if (*(ptr + 1) == '>') {
                        parse_x = 1;
                        listAppend(l, x, y);
                        x = 0;
                        y = 0;
                    }
                    break;
                case ',':
                    parse_x = 0;
                    break;
                default: {
                    int tmp_int = 0;
                    if (isdigit(*ptr)) {
                        while (isdigit(*ptr)) {
                            tmp_int = tmp_int * 10 + (*ptr - 48);
                            ++ptr;
                        }
                        if (parse_x == 1) {
                            x = (tmp_int - X_OFFSET);
                            if (x > maxX) {
                                maxX = x;
                            } else if (minX > x) {
                                minX = x;
                            }
                        } else {
                            y = tmp_int;
                            if (y > maxY) {
                                maxY = y;
                            } else if (minY > y) {
                                minY = y;
                            }
                        }
                        --ptr;
                    }
                    break;
                }
            }
            ptr++;
        }
        listAppend(l, x, y);
    }

    /*
    for (int i = 0; i < list_count; ++i) {
        listPrint(lists[i]);
    }
    */

    g->rows = WORLD_HEIGHT+2;
    g->columns = WORLD_WIDTH;

    gridPlot(g, lists, list_count);

    int count = 0;
    while (1) {
        int rest[2];
        simulateSand(g, rest);
        if (rest[0] == -1 && rest[1] == -1) {
            printf("solution1: %d\n", count);
            break;
        }
        count++;
    }

    plotLine(g, 0, maxY + 2, WORLD_WIDTH - 1, maxY + 2, '_');
    gridPrint(g);
    while (1) {
        int rest[2] = {0};
        simulateSand(g, rest);
        if (rest[0] == 500 - X_OFFSET && rest[1] == 0) {
            printf("solution2: %d\n", count+1);
            break;
        }
        count++;
    }
    // gridPrint(g);

    fclose(fp);
    gridRelease(g);
    return 0;
}
