#include <stdlib.h>
#include <stdio.h>

#include "list.h"

typedef struct dist {
    int name;
    int distance;
} dist;

dist *distNew(int name, int distance) {
    dist * d = malloc(sizeof(dist));
    d->name = name;
    d->distance = distance;
    return d;
}

void another(list *l) {
    lNode *ln = l->root;
    dist *d = (dist *)ln->val;
    printf("name: %d, distance: %d\n", d->name, d->distance);
}

int main(void) {
    list *l = listNew();
    listAppend(l, 10);
    listAppend(l, 0);
    listAppend(l, -1);
    listAppend(l, 69);
    listAppend(l, 42);
    listAppend(l, 200);
    listAppend(l, 0);
    listAppend(l, 2);
    listAppend(l, 2);
    listAppend(l, 88);
    listPrint(l);

    int actual = 0;
    list **combos = listGetAllCombinations(l,3,100000,&actual);
    printf("got combos\n");
    for (int i = 0; i < actual; ++i) {
        listPrint(combos[i]);
    }


    list *a = listNew();



    dist *d = distNew(66, 90);
    listAppend(a,(long) d);
    another(a);

    listHeapSort(l);
    listPrint(l);
}
