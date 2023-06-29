#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isTerm(ch) ((ch) == '\0' || (ch) == '\n')
typedef char i8;

typedef struct treeMap {
    int cols;
    int rows;
    int size;
    i8 *entries;
    i8 *vis;
} treeMap;

treeMap *treeMapFromFile(char *filename) {
    treeMap *tm = malloc(sizeof(treeMap));
    FILE *fp = fopen(filename, "r");
    fseek(fp,0,SEEK_END);
    unsigned long size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    char *buffer = malloc(sizeof(char) * size + 1);

    assert(fread(buffer,sizeof(i8),size,fp) == size);

    buffer[size] = '\0';
    int i = 0;
    tm->entries = buffer;
    tm->cols = 0;
    tm->rows = 0;

    while (buffer[i] != '\0') {
        if (buffer[i] == '\n') {
            tm->rows++;
        } else if (tm->rows == 0) {
            tm->cols++;
        }
        i++;
    }

    char *p1 = buffer;
    char *p2 = buffer;

    while (*p2) {
        if (*p2 == '\n') {
            p2++;
        }
        *p1++ = *p2++;
    }
    *p1 = '\0';
    tm->size = tm->rows * tm->cols;
    tm->vis = malloc(sizeof(i8) * tm->size);

    fclose(fp);

    return tm;
}

void treeMapRelease(treeMap *tm) {
    free(tm->vis);
    free(tm->entries);
    free(tm);
}

i8 treeMapGet(treeMap *tm, int x, int y) {
    if (x >= tm->rows || x < 0 || y < 0 || y >= tm->cols) {
        return -1;
    }
    int idx = (y * tm->cols) + x;
    i8 ret = tm->entries[idx];
    return ret;
}

void treeMapPrint(treeMap *tm, int print_entries) {
    if (print_entries) {
        for (int y = 0; y < tm->rows; ++y) {
            for (int x = 0; x < tm->cols; ++x) {
                printf("%c", treeMapGet(tm,x,y));
            }
            printf("\n");
        }
    }
    printf("Marked: \n");
    for (int y = 0; y < tm->rows; ++y) {
        for (int x = 0; x < tm->cols; ++x) {
            int idx = (y * tm->cols) + x;
            printf("%c", tm->vis[idx] == '1' ? '1' : '0');
        }
        printf("\n");
    }
}

void treeMapPrintScanLocation(treeMap *tm, int px, int py) {
    int scanloc = (py * tm->cols) + px;
    for (int y = 0; y < tm->rows; ++y) {
        for (int x = 0; x < tm->cols; ++x) {
            int idx = (y * tm->cols) + x;
            if (idx == scanloc) {
                printf("%c", '#');
            } else {
                printf("%c", tm->vis[idx] == '1' ? '1' : '0');
            }
        }
        printf("\n");
    }
}

void treeMapMarkVisible(treeMap *tm, int px, int py,
                                     int vx, int vy,
                                     int nx, int ny)
{
    int bx, by;
    int steps = tm->size;
    while (steps--) {
        bx = px;
        by = py;
        i8 max = 0;
        i8 c;
        while ((c = treeMapGet(tm,px,py)) != -1) {
            if (c > max) {
                tm->vis[py*tm->cols+px] = '1';
                max = c;
            }
            // treeMapPrintScanLocation(tm,x,y);
            // printf("--\n");
            px += vx;
            py += vy;
        }
        px = bx+nx;
        py = by+ny;
    }
}

int solve1(treeMap *tm) {
    int visible = 0;
    i8 tree;

    treeMapMarkVisible(tm,0,0,
            1,0,
            0,1);
    treeMapMarkVisible(tm,tm->cols-1,0,
            -1,0,
            0,1);
    treeMapMarkVisible(
            tm,0,0,
            0,1,
            1,0);
    treeMapMarkVisible(
            tm,0,tm->rows-1,
            0,-1,
            1,0);

    treeMapPrint(tm,1);
    for (int i = 0; i < tm->size; ++i) {
        if (tm->vis[i] == '1') {
            visible++;
        }
    }

    return visible;
}

int treeMapVisible(treeMap *tm, int px, int py, int vx, int vy) {
    int drawdistance = 0;
    int height = treeMapGet(tm,px,py);
    while (1) {
        px += vx;
        py += vy;
        int cur = treeMapGet(tm,px,py);
        if (cur == -1) {
            break;
        }
        drawdistance++;
        if (cur >= height) {
            break;
        }
    }
    return drawdistance;
}

int treeMapScenicView(treeMap *tm, int x, int y) {
    return treeMapVisible(tm,x,y,1,0)
         * treeMapVisible(tm,x,y,-1,0)
         * treeMapVisible(tm,x,y,0,1)
         * treeMapVisible(tm,x,y,0,-1);
}

int solve2(treeMap *tm) {
    int best = 0;
    for (int x = 0; x < tm->cols; ++x) {
        for (int y = 0; y < tm->rows; ++y) {
            int score = treeMapScenicView(tm,x,y);
            if (score > best) {
                best = score;
            }
        }
    }

    return best;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    treeMap *tm = treeMapFromFile(argv[1]);
    printf("visible trees: %d\n", solve1(tm));
    printf("Best scenic score: %d\n", solve2(tm));

    treeMapRelease(tm);
}
