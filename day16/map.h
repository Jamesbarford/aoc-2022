#ifndef __MAP_H
#define __MAP_H


#define SLAB_SIZE (1 << 12)


typedef struct mNode {
    int key;
    void *value;
    struct mNode *next;
} mNode;

typedef void (MapPrintValue)(void*);
typedef void (MapPrintKey)(int);
typedef void (MapFree)(void*);
typedef int (MapCmp)(mNode *, mNode *);

typedef struct mapType {
    MapPrintValue *printValue;
    MapPrintKey *printKey;
    MapFree *freeValue;
  //  MapCmp *cmp;
} mapType;

typedef struct map {
    int len;
    mapType *type;
    mNode **slab;
} map;

#define mapSetPrintValue(m,f) ((m)->type->printValue = (f))
#define mapSetPrintKey(m,f) ((m)->type->printValue = (f))
#define mapSetFree(m,f) ((m)->type->free = (f))

#define mapPrintKey(m,k) ((m)->type->printKey ? (m)->type->printKey((k)) : (void)k)
#define mapPrintValue(m,v) ((m)->type->printValue ? (m)->type->printValue((v)) : (void)v)
#define mapFreeValue(m,v) ((m)->type->freeValue ? (m)->type->freeValue(v) : (void)v)
//#define mapCmp(m,n1,n2) ((m)->type->cmp(n1,n2))

map *mapNew(mapType *type);
void *mapGet(map *m, int key);
int mapHas(map *s, int key);
void mapAdd(map *m, int key, void *value);
void mapDelete(map *m, int key);
void mapClear(map *m);
void mapRelease(map *m);
void mapPrint(map *m);
int *mapKeys(map *m, int *outlen);

#endif
