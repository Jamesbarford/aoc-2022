#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct map {
    int rows;
    int columns;
    char *grid;
} map;

typedef struct point {
    int x; 
    int y;
} point; 

map *mapNew(int rows, int columns) {
    map *m = malloc(sizeof(map));
    m->rows = rows;
    m->columns = columns;
    m->grid = malloc(sizeof(char) * (rows * columns));
    memset(m->grid, '.', rows*columns);
    return m;
}

point *pointArrayNew(int count, int start_x, int start_y) {
    point *arr = malloc(sizeof(point) * count);
    for (int i = 0; i < count; ++i) {
        point *p = &arr[i];
        p->x = start_x;
        p->y = start_y;
    }
    return arr;
}

char mapGet(map *m, int x, int y) {
    int idx = y*m->columns+x;
    if (x < 0 || y < 0 || x > m->columns || y>m->rows) {
        return -1;
    }
    int ret = m->grid[idx];
    return ret;
}

void mapPrint(map *m) {
    for (int y = 0; y < m->rows; ++y) {
        for (int x = 0; x < m->columns; ++x) {
            printf("%c", mapGet(m,x,y));
        }
        printf("\n");
    }
}

void mapPrintPlotHeadAndTail(map *m, int hx,int hy, int tx, int ty) {
    int headidx = hy*m->columns+hx;
    int tailidx = ty*m->columns+tx;
    int idx = 0;
    for (int y = 0; y < m->rows; ++y) {
        for (int x = 0; x < m->columns; ++x) {
            idx = y*m->columns+x;
            if (idx == headidx) {
                printf("%c", 'H');
            } else if (idx == tailidx) {
                printf("%c", 'T');
            } else {
                printf("%c", m->grid[idx]);
            }
        }
        printf("\n");
    }
    printf("===\n");
}

void mapSet(map *m, int x, int y) {
    int idx = y*m->columns+x;
    if (x < 0 || y < 0 || x >= m->columns || y>=m->rows) {
        return;
    }
    m->grid[idx] = '#';
}

void mapRelease(map *m) {
    free(m->grid);
    free(m);
}

int checkProximity(int x1, int y1, int x2, int y2, int vx, int vy) {
    return (x2 + vx == x1) && (y2 + vy == y1);
}

int pointInProximity(point *p1, point *p2) {
    return checkProximity(p1->x,p1->y,p2->x,p2->y,1,0)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,-1,0)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,0,1)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,0,-1)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,1,1)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,-1,-1)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,1,-1)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,-1,1)
        || checkProximity(p1->x,p1->y,p2->x,p2->y,0,0);
}

/* Solves problem 1 */
void moveHeadAndTail(map *m, point *h,
                             point *t,
                             int vx, int vy, int iterations)
{
    int px, py;
    for (int i = 0; i < iterations; ++i) {
        px = h->x;
        py = h->y;
        h->x += vx;
        h->y += vy;
        
        if (pointInProximity(h,t)) {
            mapSet(m, t->x, t->y);
        } else {
            t->x = px;
            t->y = py;
            mapSet(m, t->x, t->y);
        }
    }
}

void swap(int *x, int *y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

void moveHeadAndTail2(map *m, point *arr,
                      int count,
                      int vx, int vy, int iterations)
{

    point *h = &arr[0];
    point *prev, *t;

    int dx,dy;
    int px = h->x;
    int py = h->y;

    while (iterations--) {
        h->x += vx;
        h->y += vy;

        for (int j = 0; j < count-1; ++j) {
            prev = &arr[j];
            t = &arr[j + 1];
            dx = prev->x - t->x;
            dy = prev->y - t->y;

            if ((dx == 2 || dx == -2) && (dy == 2 || dy == -2)) {
                dx /= 2;
                dy /= 2;
                
                t->x += dx;
                t->y += dy;
            } else if (dx == 2) {
                t->x++;
                t->y = prev->y;
            } else if (dx == -2) {
                t->x--;
                t->y = prev->y;
            } else if (dy == 2) {
                t->y++;
                t->x = prev->x;
            } else if (dy == -2) {
                t->y--;
                t->x = prev->x;
            } else {
                break;
            }
        }
        /* The tail */
        mapSet(m, arr[count-1].x, arr[count-1].y);
    }
}

int mapCalculateTailPostions(map *m) {
    int acc = 0;
    for (int i = 0; i < m->rows*m->columns; ++i) {
        if (m->grid[i] == '#') {
            acc++;
        }
    }
    return acc;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    FILE *fp = fopen(argv[1], "r");
    char buffer[100];
    char dir;
    int iterations;
    int dims = 1000;
    int arr_count = 2;
    map *m = mapNew(dims,dims);
    int start_x = m->columns/2;
    int start_y = m->rows/2;
    point *arr = pointArrayNew(arr_count,start_x,start_y);

    while (fgets(buffer, sizeof(buffer), fp)) {
        sscanf(buffer, "%c %d", &dir, &iterations);
        switch (dir) {
        case 'U': moveHeadAndTail2(m,arr,arr_count, 0,-1,iterations); break;
        case 'D': moveHeadAndTail2(m,arr,arr_count, 0, 1,iterations); break;
        case 'L': moveHeadAndTail2(m,arr,arr_count,-1, 0,iterations); break;
        case 'R': moveHeadAndTail2(m,arr,arr_count, 1, 0,iterations); break;
        default:
            fprintf(stderr, "INVALID DIRECTION: %c\n", dir);
            exit(EXIT_FAILURE);
        }
    }
    //mapPrint(m);
    int positions = mapCalculateTailPostions(m);
    printf("p1: %d\n", positions);
    

    fclose(fp);
    mapRelease(m);
}
