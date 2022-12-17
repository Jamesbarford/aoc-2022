#ifndef __HMAP_H__
#define __HMAP_H__

#define HM_MIN_CAPACITY (1 << 16)
#define HM_LOAD (0.67)
#define HM_ERR (-1)
#define HM_OK (1)
#define HM_FOUND (-4)
#define HM_NOT_FOUND (3)

typedef int hmapKeyCompare(void *k1, unsigned int h1, void *k2, unsigned int h2);
typedef unsigned int hmapHashFunction(void *);

typedef struct hmapType {
    hmapKeyCompare *keycmp;
    hmapHashFunction *hashFn;
    void (*freekey)(void *);
    void (*freevalue)(void *);
} hmapType;

typedef struct hmapEntry {
    void *key;
    void *value;
    unsigned int hash;
    struct hmapEntry *next;
} hmapEntry;

typedef struct hmap {
    unsigned int size;
    unsigned int capacity;
    unsigned int mask;
    unsigned int rebuild_threashold;
    int fixedsize;
    hmapEntry **entries;
    hmapType *type;
} hmap;

typedef struct hmapIterator {
    unsigned int idx;
    hmap *hm;
    hmapEntry *cur;
} hmapIterator;

#define hmapHash(h, k) ((h->type)->hashFn((k)))
#define hmapSetFreeKey(h, fn) ((h)->type->freekey = (fn))
#define hmapSetFreeValue(h, fn) ((h)->type->freevalue = (fn))

hmap *hmapNew(hmapType *type);
hmap *hmapNewFixed(hmapType *type, unsigned int capacity);
void hmapRelease(hmap *hm);

int hmapContains(hmap *hm, void *key);
int hmapAdd(hmap *hm, void *key, void *value);
/* Return entry for user to free*/
hmapEntry *hmapDelete(hmap *hm, void *key);
hmapEntry *hmapGetEntry(hmap *hm, void *key);

void hmapIteratorInit(hmapIterator *iter, hmap *hm);
int hmapIteratorGetNext(hmapIterator *iter);

/* Hashing functions */
unsigned int hmapHashString(void *key);
unsigned int hmapHashNumber(void *s);

/* Key compare functions */
int hmapNumCmp(void *k1, unsigned int h1, void *k2, unsigned int h2);
int hmapStrCmp(void *k1, unsigned int h1, void *k2, unsigned int h2);

static inline int hmapKeycmp(hmap *hm, void *k1, unsigned int h1, void *k2, unsigned int h2) {
    return hm->type->keycmp(k1, h1, k2, h2);
}

static inline void hmapKeyRelease(hmap *hm, void *key) {
    hm->type->freekey(key);
}

static inline void hmapValueRelease(hmap *hm, void *key) {
    hm->type->freevalue(key);
}

static inline void *hmapGet(hmap *hm, void *key) {
    hmapEntry *he;

    if ((he = hmapGetEntry(hm, key)) == NULL) {
        return NULL;
    }

    return he->value;
}

#endif
