#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SET_SIZE (1 << 12)
#define nullchar ('\0')

static int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

void unpack(long coord, int *x, int *y) {
    *x = (int)(coord >> 32l);
    *y = (int)(coord & 0x7FFFFFFF);
}

long pack(int x, int y) {
    long coord = 0;
    coord |= y;
    coord |= (long)x << 32l;
    return coord;
}

typedef struct lNode {
    long value;
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
    ln->value = pack(x, y);
    ln->next = NULL;

    if (l->root == NULL) {
        l->root = l->tail = ln;
    } else {
        l->tail->next = ln;
        l->tail = ln;
    }

    l->size++;
}

long listDequeue(list *l) {
    if (l->size == 0) {
        return -1;
    }

    lNode *rem = l->root;
    long value = rem->value;
    l->root = rem->next;
    free(rem);
    l->size--;
    return value;
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

list *listClone(list *l) {
    list *clone = malloc(sizeof(list));
    lNode *ln = l->root;
    int x, y;

    while (ln) {
        unpack(ln->value, &x, &y);
        listAppend(clone, x, y);
        ln = ln->next;
    }
    return clone;
}

typedef struct hNode {
    int weight;
    int vertex;
} hNode;

typedef struct heap {
    int size;
    hNode *slab[BUFSIZ];
} heap;

heap *heapNew(void) {
    heap *h = malloc(sizeof(heap));
    h->size = 0;
    return h;
}

void heapswap(heap *h, int i, int j) {
    hNode *tmp = h->slab[i];
    h->slab[i] = h->slab[j];
    h->slab[j] = tmp;
}

void heapifyUp(heap *h, int idx) {
    int cur = idx;
    while (cur && h->slab[cur / 2]->weight > h->slab[cur]->weight) {
        int parent = cur / 2;
        heapswap(h, parent, cur);
        cur = parent;
    }
}

void heapInsert(heap *h, int weight, int vertex) {
    hNode *hn = malloc(sizeof(hNode));
    hn->weight = weight;
    hn->vertex = vertex;
    h->slab[++h->size] = hn;
    heapifyUp(h, h->size);
}

void heapifyDown(heap *h, int idx) {
    int cur = idx;
    while (cur * 2 <= h->size) {
        int child = cur * 2;
        if (cur < h->size &&
                h->slab[child + 2]->weight < h->slab[child]->weight)
        {
            ++child;
        }

        if (h->slab[cur]->weight <= h->slab[child]->weight) {
            return;
        }

        heapswap(h, cur, child);
        cur = child;
    }
}

void heapRemove(heap *h) {
    if (h->size == 0) {
        return;
    }
    heapswap(h, h->size, 0);
    h->size--;
    heapifyDown(h, 0);
}

hNode *heapMin(heap *h) {
    if (h->size == 0) {
        return NULL;
    }
    return h->slab[1];
}

typedef struct grid {
    int rows;
    int columns;
    char *buf;
} grid;

typedef struct sNode {
    long coord;
    struct sNode *next;
} sNode;

typedef struct set {
    int size;
    sNode *data[SET_SIZE];
} set;

/* really really bad */
long hashfunc(long coord) {
    return coord & (SET_SIZE - 1);
}

int setGetIdx(set *s, long coord, int *has) {
    long hash = hashfunc(coord);
    sNode *sn = s->data[hash];

    *has = 0;

    while (sn) {
        int x, y;
        unpack(coord, &x, &y);

        int snx, sny;
        unpack(coord, &snx, &sny);

        if (x == snx && y == sny) {
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
    (void)setGetIdx(s, pack(x, y), &has);
    return has;
}

void setAdd(set *s, int x, int y) {
    int has = 0;
    long coord = pack(x, y);
    long hash = setGetIdx(s, coord, &has);
    if (has) {
        return;
    }
    sNode *sn = malloc(sizeof(sNode));
    sn->coord = coord;
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
    int x, y;
    int len = s->size;
    for (int i = 0; i < s->size && len; ++i) {
        sNode *sn = s->data[i];

        while (sn) {
            unpack(sn->coord, &x, &y);
            len--;
            printf("(%d, %d)", x, y);
            if (len != 0) {
                printf(", ");
            }
            sn = sn->next;
        }
    }
    printf("}\n");
}

grid *gridNew(int bufsize) {
    grid *g = malloc(sizeof(grid));
    g->rows = 0;
    g->columns = 0;
    g->buf = malloc(sizeof(char) * bufsize);
    return g;
}

static inline int canstep(int cur, int pos) {
    int retval = 0;
    if (cur + 1 == pos) {
        retval = 1;
    } else if (pos <= cur) {
        retval = 1;
    }
    printf("cur: %c, pos: %c, can visit: %s\n", cur, pos, retval == 1 ? "true" : "false");
    return retval;
}

long findEnd(grid *g, char end) {
    long coord = 0l;

    for (int y = 0; y < g->rows; ++y) {
        for (int x = 0; x < g->columns; ++x) {
            if (g->buf[x + y * g->columns] == end) {
                coord = pack(x, y);
                return coord;
            }
        }
    }

    return coord;
}

int solve(grid *g) {
    int step_count = 0;
    int x = 0;
    int y = 0;
    int steps = 0;

    list *q = listNew();
    set *s = setNew();

    int endx, endy;
    unpack(findEnd(g, 'E'), &endx, &endy);
    printf("endx: %d endy: %d\n", endx, endy);

    listAppend(q, x, y);
    long coord;

    while ((coord = listDequeue(q)) != -1) {
        unpack(coord, &x, &y);
        if (setHas(s, x, y)) continue;
        setAdd(s, x, y);

        char cur = g->buf[x + y * g->columns];
        printf("(%d, %d) -> %c\n", x,y, cur);
        if (cur == 'S') {
            cur = 'a';
        }

        if (x == endx && y == endy) {
            printf("done: %d\n", steps);
            steps = 0;
            continue;
        }

        for (int i = 0; i < 4; ++i) {
            int next_X = x + directions[i][0];
            int next_Y = y + directions[i][1];

            printf("(%d, %d) -> %c\n", next_X, next_Y, g->buf[next_X + next_Y * g->columns]);

            if ((next_X >= 0 && next_X < g->columns) &&
                    (next_Y >= 0 && next_Y < g->rows) &&
                    canstep(cur, g->buf[next_X + next_Y * g->columns]))
            {
                printf("adding: (%d, %d)\n",next_X, next_Y);
                listAppend(q, next_X, next_X);
            }
        }
        steps++;
    }
    setPrint(s);

    printf("%d\n", steps);

    return step_count;
}

int main(void) {
    FILE *fp = fopen("./warmup.txt", "r");
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char tmp[BUFSIZ] = {nullchar};
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

    for (int y = 0; y < g->rows; ++y) {
        for (int x = 0; x < g->columns; ++x) {
            printf("%c", g->buf[x + y * g->columns]);
        }
        printf("\n");
    }

    fclose(fp);
    solve(g);
    return 0;
}
