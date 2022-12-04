#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

#define SLAB_SIZE (128)
#define toint(p) ((p)-48)

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
            printf("%d ", hn->value);
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
            return 1;
        }
        hn = hn->next;
    }

    return 0;
}

hashset *hashsetIntersection(hashset *h1, hashset *h2) {
    hashset *hs = hashsetNew();
    for (int i = 0; i < SLAB_SIZE; ++i) {
        hNode *hn = h1->slab[i];
        while (hn) {
            if (hashsetHas(h2, hn->value)) {
                hashsetInsert(hs, hn->value);
            }
            hn = hn->next;
        }
    }

    return hs;
}

void hashsetClear(hashset *h) {
    if (h) {
        for (int i = 0; i < SLAB_SIZE; ++i) {
            hNode *hn = h->slab[i];
            hNode *cur = hn;
            while (cur) {
                cur = hn->next;
                free(hn);
                hn = cur;
            }
            h->slab[i] = NULL;
        }
    }
}

void hashsetRelease(hashset *h) {
    if (h) {
        hashsetClear(h);
        free(h->slab);
        free(h);
    }
}

int hashsetPartialIntersection(hashset *h1, hashset *h2) {
    int retval = 1;
    hashset *i1 = hashsetIntersection(h1, h2);
    hashset *i2 = hashsetIntersection(h2, h1);

    retval = i1->size > 0 || i2->size > 0;

    hashsetRelease(i1);
    hashsetRelease(i2);
    return retval;
}

void expandToHashset(hashset *h1, int start, int end) {
    for (int i = start; i <= end; ++i) {
        hashsetInsert(h1, i);
    }
}

/***
    2-4,6-8
    2-3,4-5
    5-7,7-9
    2-8,3-7
    6-6,4-6
    2-6,4-8
 */
int solve(rFile *rf) {
    char *ptr = rf->buf;
    int acc = 0;
    int isstart = 1;
    int start = 0;
    int end = 0;
    hashset *h1 = hashsetNew();
    hashset *h2 = hashsetNew();

    while (*ptr) {
        switch (*ptr) {
            case '-':
                isstart = 0;
                break;

            case ',':
                expandToHashset(h1, start, end);
                start ^= start;
                end ^= end;
                isstart = 1;
                break;

            case '\n':
                expandToHashset(h2, start, end);
                start ^= start;
                end ^= end;
                isstart = 1;
                acc += hashsetPartialIntersection(h1, h2);
                hashsetClear(h1);
                hashsetClear(h2);
                break;

            default:
                if (isdigit(*ptr)) {
                    if (isstart) {
                        start = start * 10 + toint(*ptr);
                    } else {
                        end = end * 10 + toint(*ptr);
                    }
                }
                break;
        }
        ++ptr;
    }

    hashsetRelease(h1);
    hashsetRelease(h2);

    return acc;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    printf("%d\n", solve(rf));
    rFileRelease(rf);
}
