#include <stdio.h>
#include <stdlib.h>

#define SET_SIZE (1 << 12)

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

int main(void) {
    int x = 42;
    int y = 69;
    long coord = 0;
    set *s = setNew();

    setAdd(s, 0, 1);
    printf("set has: %s\n", setHas(s, 1, 0) ? "true" : "false");
    setAdd(s, 1, 0);
    setPrint(s);

    coord |= (long)42 << 32l;
    coord |= y;

    printf("%d\n", 't' < 'b');

    printf("%ld -> x:%d, y:%d\n", coord, (int)(coord >> 32l),
            (int)coord & 0x7FFFFF);
}
