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

hashset *hashsetIntersectionMany(hashset **sets, int count) {
    hashset *current_intersection = sets[0];
    hashset *tmp;

    for (int i = 1; i < count; ++i) {
        hashset *h2 = sets[i];
        tmp = current_intersection;
        current_intersection = hashsetIntersection(tmp, h2);
        if (i != 1) {
            hashsetRelease(tmp);
        }
    }

    return current_intersection;
}

int hashsetSum(hashset *h) {
    int acc = 0;
    for (int i = 0; i < SLAB_SIZE; ++i) {
        hNode *hn = h->slab[i];
        while (hn) {
            acc += hn->value;
            hn = hn->next;
        }
    }

    return acc;
}

int solve(rFile *rf) {
    char *ptr = rf->buf;
    int acc = 0;
    int line_count = 0;
    hashset *sets[3];
    hashset *tmp;

    for (int i = 0; i < 3; ++i) {
        sets[i] = hashsetNew();
    }

    while (*ptr) {
        if (*ptr == '\n') {
            line_count++;
            if (line_count == 3) {
                tmp = hashsetIntersectionMany(sets, 3);
                hashsetPrint(tmp);
                acc += hashsetSum(tmp);

                line_count = 0;
                for (int i = 0; i < 3; ++i) {
                    hashsetClear(sets[i]);
                }
                hashsetRelease(tmp);

                tmp = NULL;
            }
        } else {
            hashsetInsert(sets[line_count], toPriority(*ptr));
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
