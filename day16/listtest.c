#include "list.h"

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

    listHeapSort(l);
    listPrint(l);
}
