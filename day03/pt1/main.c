/**
 * Massively over engineered
 */
#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

#define SLAB_SIZE (128)
#define toPriority(x) ((x) >= 'a' && (x) <= 'z' ? (x)-96 : (x)-38)
#define tochar(x) ((x) >= 1 && (x) <= 27 ? (x) + 96 : (x) + 38)

typedef struct hNode {
    int value;
    struct hNode *next;
} hNode;

typedef struct hashset {
    unsigned int size;
    hNode **slab;
} hashset;

hashset *hashsetNew(void) {
    hashset *hs = malloc(sizeof(hashset));
    hs->size = 0;
    hs->slab = calloc(SLAB_SIZE, sizeof(hNode *));
    return hs;
}

void hashsetPrint(hashset *hs) {
    printf("{ ");
    for (int i = 0; i < SLAB_SIZE; ++i) {
        hNode *hn = hs->slab[i];
        while (hn) {
            printf("%c ", tochar(hn->value));
            hn = hn->next;
        }
    }

    printf("}\n");
}

/* worlds worst hash function */
static inline unsigned int hashfunc(int value) {
    return value & SLAB_SIZE - 1;
}

void hashsetInsert(hashset *hs, int value) {
    unsigned int hash = hashfunc(value);

    if (hs->slab[hash] == NULL) {
        hNode *hn = malloc(sizeof(hNode));
        hn->value = value;
        hn->next = hs->slab[hash];
        hs->slab[hash] = hn;
    } else {
        hNode *n = hs->slab[hash];

        while (n) {
            if (n->value == value) {
                return;
            }
            n = n->next;
        }

        hNode *new = malloc(sizeof(hNode));
        new->value = value;
        new->next = hs->slab[hash];
        hs->slab[hash] = new;
    }
    hs->size++;
}

int hashsetHas(hashset *h, int value) {
    hNode *hn = h->slab[hashfunc(value)];

    while (hn) {
        if (hn->value == value) {
            return hn->value;
        }
        hn = hn->next;
    }

    return -1;
}

int hashsetIntersection(hashset *h1, hashset *h2) {
    int acc = 0;
    for (int i = 0; i < SLAB_SIZE; ++i) {
        hNode *hn = h1->slab[i];
        while (hn) {
            acc = hashsetHas(h2, hn->value);
            if (acc != -1) {
                goto out;
            } else {
                hn = hn->next;
            }
        }
    }

out:
    return acc;
}

void hashsetRelease(hashset *h) {
    if (h) {
        for (int i = 0; i < SLAB_SIZE; ++i) {
            hNode *hn = h->slab[i];
            hNode *cur = hn;
            while (cur) {
                cur = hn->next;
                free(hn);
                hn = cur;
            }
        }
        free(h->slab);
        free(h);
    }
}

int calculatebuf(int *buf, int len) {
    hashset *h1 = hashsetNew();
    hashset *h2 = hashsetNew();

    for (int i = 0; i < (len / 2); ++i) {
        hashsetInsert(h1, buf[i]);
    }

    for (int j = len / 2; j < len; ++j) {
        hashsetInsert(h2, buf[j]);
    }

    int val = hashsetIntersection(h1, h2);

    hashsetRelease(h1);
    hashsetRelease(h2);

    return val;
}

int solve(rFile *rf) {
    int tmp[BUFSIZ] = {0};
    char *ptr = rf->buf;
    int len = 0;
    int acc = 0;

    while (*ptr) {
        if (*ptr == '\n') {
            int t = calculatebuf(tmp, len);
            acc += t;
            len = 0;
        } else {
            tmp[len++] = toPriority(*ptr);
        }

        ptr++;
    }
    return acc;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    printf("%d\n", solve(rf));
    rFileRelease(rf);
}
