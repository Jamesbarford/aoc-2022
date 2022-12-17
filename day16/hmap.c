#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmap.h"

#define _hmapShouldRebuild(h) ((h)->size >= (h)->rebuild_threashold && (h)->fixedsize == 0)

static inline hmapEntry **hmapEntryAlloc(unsigned int capacity) {
    return (hmapEntry **)calloc(capacity, sizeof(hmapEntry *));
}

unsigned int hmapHashString(void *key) {
    char *s = (char *)key;
    unsigned int h = (unsigned int)*s;

    if (h) {
        for (++s; *s; ++s)
            h = (h << 5) - h + (unsigned int)*s;
    }

    return h;
}

unsigned int hmapHashNumber(void *s) {
    unsigned int *x = (unsigned int *)s;
    unsigned int tmp = *x;

    tmp = ((tmp >> 16) ^ tmp) * 0x45d9f3b;
    tmp = ((tmp >> 16) ^ tmp) * 0x45d9f3b;
    tmp = (tmp >> 16) ^ tmp;
    return tmp;
}

int hmapNumCmp(void *k1, unsigned int h1, void *k2, unsigned int h2) {
    double *n1 = (double *)k1;
    double *n2 = (double *)k2;

    return h1 == h2 && *n1 == *n2;
}

int hmapStrCmp(void *k1, unsigned int h1, void *k2, unsigned int h2) {
    char *str1 = (char *)k1;
    char *str2 = (char *)k2;

    return h1 == h2 && strcmp(str1, str2) == 0;
}

static unsigned int roundup32bit(unsigned int num) {
    num--;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    num++;
    return num;
}

static void _hmapEntryRelease(hmap *hm, hmapEntry *he, int freeHe) {
    if (he) {
        if (freeHe) {
            hmapKeyRelease(hm, he->key);
            hmapValueRelease(hm, he->value);
            free(he);
        }
    }
}

hmapType defaultType = {
        .keycmp = hmapStrCmp,
        .hashFn = hmapHashString,
        .freekey = free,
        .freevalue = free,
};

void hmapRelease(hmap *hm) {
    hmapEntry *he, *next, *cur;

    for (unsigned int i = 0; i < hm->size; ++i) {
        if ((he = hm->entries[i]) != NULL) {
            if (he->next != NULL) {
                cur = he->next;

                while (cur) {
                    next = cur->next;
                    _hmapEntryRelease(hm, cur, 1);
                    cur = next;
                }
            }
            _hmapEntryRelease(hm, he, 1);
        }
    }

    hm->size = 0;
    hm->mask = 0;
    hm->rebuild_threashold = 0;
    free(hm->entries);
    free(hm);
}

static hmap *_hmapCreate(hmapType *type, int fixedsize, unsigned int startCapacity) {
    hmap *hm;

    if ((hm = (hmap *)malloc(sizeof(hmap))) == NULL) {
        return NULL;
    }

    if ((hm->entries = hmapEntryAlloc(HM_MIN_CAPACITY)) == NULL) {
        free(hm);
        return NULL;
    }

    hm->type = type == NULL ? &defaultType : type;
    hm->capacity = startCapacity;
    hm->size = 0;
    hm->mask = hm->capacity - 1;
    hm->rebuild_threashold = ~~((unsigned int)(HM_LOAD * hm->capacity));
    hm->fixedsize = fixedsize;

    return hm;
}

/* Create new hashtable with specified type */
hmap *hmapNew(hmapType *type) {
    return _hmapCreate(type, 0, HM_MIN_CAPACITY);
}

/* Create new fixedsize hashtable with specified type */
hmap *hmapNewFixed(hmapType *type, unsigned int capacity) {
    return _hmapCreate(type, 1, roundup32bit(capacity));
}

static int _hmapExpand(hmap *hm) {
    unsigned int i;
    int idx;
    hmapEntry **newEntries, **oldEntries;
    unsigned int newCapacity, newMask, newRebuildThreashold, size;

    size = hm->size;

    newCapacity = hm->capacity << 1;
    newMask = newCapacity - 1;
    newRebuildThreashold = ~~((unsigned int)(HM_LOAD * newCapacity));

    newEntries = hmapEntryAlloc(newCapacity);
    if (newEntries == NULL)
        return HM_ERR;

    for (i = 0; i < hm->capacity && size > 0; ++i) {
        hmapEntry *he, *nextHe;

        if (hm->entries[i] == NULL)
            continue;

        he = hm->entries[i];
        while (he) {
            idx = he->hash & newMask;
            nextHe = he->next;
            he->next = newEntries[idx];
            newEntries[idx] = he;
            size--;
            he = nextHe;
        }
    }

    oldEntries = hm->entries;
    hm->entries = newEntries;
    hm->mask = newMask;
    hm->capacity = newCapacity;
    hm->rebuild_threashold = newRebuildThreashold;
    free(oldEntries);
    return HM_OK;
}

static int _hmapGetIndex(hmap *hm, void *key, unsigned int hash) {
    hmapEntry *he;
    int idx;

    if (_hmapShouldRebuild(hm)) {
        _hmapExpand(hm);
    }

    idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (hmapKeycmp(hm, key, hash, he->key, he->hash)) {
            return HM_FOUND;
        }
        he = he->next;
    }

    return idx;
}

hmapEntry *hmapGetEntry(hmap *hm, void *key) {
    hmapEntry *he;
    unsigned int idx, hash;

    hash = hmapHash(hm, key);
    idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (hmapKeycmp(hm, key, hash, he->key, he->hash)) {
            return he;
        }
        he = he->next;
    }

    return NULL;
}

int hmapContains(hmap *hm, void *key) {
    unsigned int hash = hmapHash(hm, key);
    if (_hmapGetIndex(hm, key, hash) == HM_FOUND) {
        return HM_FOUND;
    }
    return HM_NOT_FOUND;
}

int hmapAdd(hmap *hm, void *key, void *value) {
    int idx;
    unsigned int hash;
    hmapEntry *newHe;

    hash = hmapHash(hm, key);

    if ((idx = _hmapGetIndex(hm, key, hash)) == HM_FOUND) {
        return HM_FOUND;
    }

    if ((newHe = (hmapEntry *)malloc(sizeof(hmapEntry))) == NULL) {
        return HM_ERR;
    }

    newHe->next = hm->entries[idx];
    newHe->key = key;
    newHe->value = value;
    newHe->hash = hash;
    hm->entries[idx] = newHe;
    hm->size++;

    return HM_OK;
}

/**
 * Remove the entry from the hashtable, the caller frees the value, key
 * and entry object.
 */
hmapEntry *hmapDelete(hmap *hm, void *key) {
    unsigned int hash, idx;
    hmapEntry *he, *heprev;

    heprev = NULL;
    hash = hmapHash(hm, key);
    idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (hmapKeycmp(hm, key, hash, he->key, he->hash)) {
            if (heprev) {
                heprev->next = he->next;
            } else {
                hm->entries[idx] = he->next;
            }
            hm->size--;
            return he;
        } else {
            heprev = he;
            he = he->next;
        }
    }

    return NULL;
}

void hmapIteratorInit(hmapIterator *iter, hmap *hm) {
    iter->hm = hm;
    iter->cur = NULL;
    iter->idx = 0;
}

int hmapIteratorGetNext(hmapIterator *iter) {
    unsigned int idx;

    while (iter->idx < iter->hm->capacity) {
        if (iter->cur == NULL || iter->cur->next == NULL) {
            idx = iter->idx;
            iter->idx++;

            if (iter->hm->entries[idx] != NULL) {
                iter->cur = iter->hm->entries[idx];
                return 1;
            }
        } else if (iter->cur->next != NULL) {
            iter->cur = iter->cur->next;
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}
