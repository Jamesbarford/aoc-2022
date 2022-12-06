#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "../includes/readfile.h"

#define toint(p) ((p)-48)
#define SLAB_SIZE (1 << 5)
/* Change K to solve the problem */
#define K (14)

typedef struct hNode {
    char value;
    struct hNode *next;
} hNode;

typedef struct hashset {
    int len;
    hNode **slab;
} hashset;

hashset *hashsetNew(void) {
    hashset *h = malloc(sizeof(hashset));
    h->len = 0;
    h->slab = calloc(SLAB_SIZE, sizeof(hNode *));
    return h;
}

unsigned int hashfunc(char value) {
    return value & (SLAB_SIZE - 1);
}

int hashsetIdx(hashset *h, char value, int *has) {
    unsigned int hash = hashfunc(value);
    hNode *hn = h->slab[hash];
    *has = 0;

    while (hn) {
        if (hn->value == value) {
            *has = 1;
            return hash;
        }
        hn = hn->next;
    }
    return hash;
}

int hashsetHas(hashset *h, char value) {
    int has = 0;
    (void)hashsetIdx(h, value, &has);
    return has;
}

void hashsetInsert(hashset *h, char value) {
    int has = 0;
    unsigned int hash = hashsetIdx(h, value, &has);
    if (has) {
        return;
    }
    hNode *hn = malloc(sizeof(hNode));
    hn->value = value;
    hn->next = h->slab[hash];
    h->slab[hash] = hn;
    h->len++;
}

void hashsetClear(hashset *h) {
    if (h) {
        for (int i = 0; i < SLAB_SIZE && h->len > 0; ++i) {
            hNode *hn = h->slab[i];
            hNode *next = NULL;

            while (hn) {
                next = hn->next;
                free(hn);
                hn = next;
                h->len--;
            }
            h->slab[i] = NULL;
        }
    }
}

void hashsetRelease(hashset *h) {
    if (h) {
        hashsetClear(h);
        free(h);
    }
}

void hashsetPrint(hashset *h) {
    printf("{");
    int count = 0;
    for (int i = 0; i < SLAB_SIZE; ++i) {
        hNode *hn = h->slab[i];

        while (hn) {
            printf("%c", hn->value);
            if (count != h->len - 1) {
                printf(", ");
            }
            hn = hn->next;
            count++;
        }
    }
    printf("}\n");
}

void hashsetDelete(hashset *h, char value) {
    unsigned int hash = hashfunc(value);
    hNode *hn = h->slab[hash];

    while (hn) {
        if (hn->value == value) {
            if (h->slab[hash]->value == value) {
                h->slab[hash] = NULL;
            }
            free(hn);
            h->len--;
            return;
        }
        hn = hn->next;
    }
}

int solve(rFile *rf) {
    char *ptr = rf->buf;
    int window_start = 0;
    int iter = 0;
    char tmp[K] = {'\0'};
    hashset *set = hashsetNew();
    int char_count = 0;

    while (*ptr != '\n') {
        tmp[char_count++] = *ptr;
        if (iter >= K - 1) {
            for (int i = 0; i < K; ++i) {
                hashsetInsert(set, tmp[i]);
            }
            if (set->len == K) {
                goto out;
            }
            for (int i = 1; i < K; ++i) {
                tmp[i - 1] = tmp[i];
            }
            hashsetClear(set);
            ++window_start;
            char_count = K - 1;
        }
        ++iter;
        ++ptr;
    }

out:
    hashsetPrint(set);
    hashsetRelease(set);
    return window_start + K;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    int solution = solve(rf);
    printf("%d\n", solution);

    rFileRelease(rf);
}
