#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SET_SIZE (1 << 12)
#define nullchar ('\0')

typedef struct lNode {
    int value[4];
    struct lNode *next;
} lNode;

typedef struct list {
    int size;
    lNode *head;
    lNode *tail;
} list;

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->head = l->tail = NULL;
    l->size = 0;
    return l;
}

void listAppend(list *l, int *value) {
    lNode *n = malloc(sizeof(lNode));
    for (int i = 0; i < 4; ++i) {
        n->value[i] = value[i];
    }
    n->next = NULL;

    if (l->size == 0) {
        l->tail = l->head = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    l->size++;
}

void listClear(list *l) {
    if (l) {
        lNode *cur = l->head;
        lNode *next = NULL;
        while (cur && l->size) {
            next = cur->next;
            free(cur);
            cur = next;
            l->size--;
        }
        l->head = NULL;
    }
}

void listRelease(list *l) {
    if (l) {
        listClear(l);
        free(l);
    }
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
                h->slab[child + 2]->weight < h->slab[child]->weight) {
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
    int x = coord >> 32l;
    int y = coord & 0x7FFFFFFF;
    return (x + y) & (SET_SIZE - 1);
}

int setGetIdx(set *s, long coord, int *has) {
    long hash = hashfunc(coord);
    sNode *sn = s->data[hash];

    *has = 0;

    while (sn) {
        int x = coord >> 32l;
        int y = coord & 0x7FFFFFFF;

        int snx = sn->coord >> 32l;
        int sny = sn->coord & 0x7FFFFFFF;
        if (x == snx && y == sny) {
            *has = 1;
            goto out;
        }
        sn = sn->next;
    }
out:
    return hash;
}

void setAdd(set *s, long coord) {
    int has = 0;
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

grid *gridNew(int bufsize) {
    grid *g = malloc(sizeof(grid));
    g->rows = 0;
    g->columns = 0;
    g->buf = malloc(sizeof(char) * bufsize);
    return g;
}

int solve(grid *g, int start, char sink) {
    int step_count = 0;

    return step_count;
}

int main(void) {
    FILE *fp = fopen("./warmup.txt", "r");
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = malloc(file_size * sizeof(char));
    char tmp[BUFSIZ] = {nullchar};
    grid *g = gridNew(file_size);
    int buflen = 0;
    int is_count = 1;

    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t len = strlen(tmp);
        if (tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
            --len;
        }
        g->columns = len;

        for (int i = 0; i < len; ++i) {
            buf[buflen++] = tmp[i];
        }
        g->rows++;
    }

    for (int y = 0; y < g->rows; ++y) {
        for (int x = 0; x < g->columns; ++x) {
            printf("%c", buf[x + y * g->columns]);
        }
        printf("\n");
    }

    fclose(fp);
    return 0;
}
