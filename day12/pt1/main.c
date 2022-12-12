#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SET_SIZE (1 << 12)

static int directions[4][2] = {
        {-1, 0},
        {0, 1},
        {1, 0},
        {0, -1},
};

typedef struct lNode {
    int x, y, depth;
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

void listAppend(list *l, int x, int y, int depth) {
    lNode *ln = malloc(sizeof(lNode));
    ln->x = x;
    ln->y = y;
    ln->depth = depth;
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
    while (ln) {
        printf("(%d, %d)", ln->x, ln->y);
        if (len - 1 != 0) {
            printf("->");
        }

        --len;
        ln = ln->next;
    }
    printf("\n");
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

void gridRelease(grid *g) {
    if (g) {
        free(g->buf);
        free(g);
    }
}

typedef struct sNode {
    int x, y;
    struct sNode *next;
} sNode;

typedef struct set {
    int size;
    sNode *data[SET_SIZE];
} set;

long hashfunc(int x, int y) {
    long hash = 0;
    hash |= x;
    hash |= (long)y << 32l;
    return hash & (SET_SIZE - 1);
}

int setGetIdx(set *s, int x, int y, int *has) {
    long hash = hashfunc(x, y);
    sNode *sn = s->data[hash];

    *has = 0;

    while (sn) {
        if (x == sn->x && y == sn->y) {
            *has = 1;
            goto out;
        }
        sn = sn->next;
    }
out:
    return hash;
}

int setHas(set *s, int x, int y) {
    int has = 0;
    (void)setGetIdx(s, x, y, &has);
    return has;
}

void setAdd(set *s, int x, int y) {
    int has = 0;
    long hash = setGetIdx(s, x, y, &has);
    if (has) {
        return;
    }
    sNode *sn = malloc(sizeof(sNode));
    sn->x = x;
    sn->y = y;
    sn->next = s->data[hash];
    s->data[hash] = sn;
    s->size++;
}

set *setNew(void) {
    set *s = malloc(sizeof(set));
    s->size = 0;
    return s;
}

void setRelease(set *s) {
    if (s) {
        for (int i = 0; i < SET_SIZE && s->size; ++i) {
            sNode *sn = s->data[i];
            sNode *next = NULL;
            while (sn) {
                next = sn->next;
                free(sn);
                sn = next;
                s->size--;
            }
            s->data[i] = NULL;
        }
        free(s);
    }
}

void setPrint(set *s) {
    printf("{");
    int len = s->size;
    for (int i = 0; i < s->size && len; ++i) {
        sNode *sn = s->data[i];

        while (sn) {
            len--;
            printf("(%d, %d)", sn->x, sn->y);
            if (len != 0) {
                printf(", ");
            }
            sn = sn->next;
        }
    }
    printf("}\n");
}

int partOneCond(char height, char cur) {
    return (height - cur) > 1;
}

int partTwoCond(char height, char cur) {
    return (height - cur) < -1;
}

int solve(grid *g, int start_x, int start_y, char end, int (*cond)(char, char)) {
    int depth = 0;
    lNode *ln = NULL;
    list *q = listNew();
    set *s = setNew();
    
    listAppend(q, start_x, start_y, depth);

    while ((ln = listDequeue(q))) {
        depth = ln->depth;
        char cur = gridGet(g, ln->x, ln->y);

        if (cur == end) {
            goto out;
        }

        for (int i = 0; i < 4; ++i) {
            int x2 = ln->x + directions[i][0];
            int y2 = ln->y + directions[i][1];
            char height = gridGet(g, x2, y2);

            if (height == 'E') {
                height = 'z';
            }

            if (height == -1) {
                continue;
            }

            if (setHas(s, x2, y2)) {
                continue;
            }

            if (cond(height, cur)) {
                continue;
            }

            listAppend(q, x2, y2, depth + 1);
            setAdd(s, x2, y2);
        }
        free(ln);
    }

out:
    setRelease(s);
    listRelease(q);

    return depth;
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char tmp[BUFSIZ];
    grid *g = gridNew(file_size);
    int buflen = 0;

    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t len = strlen(tmp);
        if (tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
            --len;
        }
        g->columns = len;

        for (size_t i = 0; i < len; ++i) {
            g->buf[buflen++] = tmp[i];
        }
        g->rows++;
    }

    fclose(fp);

    int x, y;
    gridFind(g, 'S', &x, &y);
    g->buf[x+y*g->columns] = 'a';
    printf("part1: %d\n", solve(g, x, y, 'E', partOneCond));

    gridFind(g, 'E', &x, &y);
    g->buf[x+y*g->columns] = 'z';
    printf("part2: %d\n", solve(g, x, y, 'a', partTwoCond));

    gridRelease(g);
    return 0;
}
