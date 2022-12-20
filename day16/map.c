#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "map.h"

static inline unsigned int hashfunc(int key) {
    return key & (SLAB_SIZE - 1);
}

map *mapNew(mapType *type) {
    map *m = malloc(sizeof(map));
    m->len = 0;
    m->slab = calloc(SLAB_SIZE, sizeof(mNode *));
    m->type = type;
    return m;
}

static int mapGetIdx(map *m, int key, int *has) {
    unsigned int hash = hashfunc(key);
    mNode *mn = m->slab[hash];
    *has = 0;

    while (mn) {
        if (mn->key == key) {
            *has = 1;
            return hash;
        }
        mn = mn->next;
    }
    return hash;
}

void *mapGet(map *m, int key) {
    unsigned int hash = hashfunc(key);
    mNode *mn = m->slab[hash];
    while (mn) {
        if (mn->key == key) {
            return mn->value;
        }
        mn = mn->next;
    }
    return NULL;
}

int mapHas(map *s, int key) {
    int has = 0;
    (void)mapGetIdx(s, key, &has);
    return has;
}

void mapAdd(map *m, int key, void *value) {
    int has = 0;
    unsigned int hash = mapGetIdx(m, key, &has);
    if (has) {
        return;
    }
    mNode *mn = malloc(sizeof(mNode));
    mn->key = key;
    mn->value = value;
    mn->next = m->slab[hash];
    m->slab[hash] = mn;
    m->len++;
}

void mapClear(map *m) {
    if (m) {
        for (int i = 0; i < SLAB_SIZE && m->len > 0; ++i) {
            mNode *mn = m->slab[i];
            mNode *next = NULL;

            while (mn) {
                next = mn->next;
                free(mn);
                mn = next;
                m->len--;
            }
            m->slab[i] = NULL;
        }
    }
}

void mapRelease(map *m) {
    if (m) {
        mapClear(m);
        free(m);
    }
}

void mapPrint(map *m) {
    printf("{\n");
    int count = 0;
    for (int i = 0; i < SLAB_SIZE; ++i) {
        mNode *mn = m->slab[i];

        while (mn) {
            printf("  ");
            mapPrintKey(m,mn->key);
            mapPrintValue(m,mn->value);

            mn = mn->next;
            count++;
        }
    }
    printf("}\n");
}

void mapDelete(map *m, int key) {
    unsigned int hash = hashfunc(key);
    mNode *cur = m->slab[hash];
    mNode *prev = NULL;

    while (cur) {
        if (cur->key == key) {
            if (prev != NULL) {
                prev->next = cur->next;
            } else {
                /* Move along by one */
                m->slab[hash] = cur->next;
            }
            mapFreeValue(m,cur->value);
            free(cur);
            m->len--;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

int *mapKeys(map *m, int *outlen) {
    int capacity = 100;
    int size = 0;
    int *keys = malloc(sizeof(int) * capacity);

    for (int i = 0; i < SLAB_SIZE; ++i) {
        mNode *mn = m->slab[i];
        while (mn) {
            if (size + 1 >= capacity) {
                capacity *= capacity;
                keys = realloc(keys, sizeof(int) * capacity);
            }
            keys[size] = mn->key;
            size++;
            mn = mn->next;
        }
    }

    *outlen = size;
    return keys;
}
