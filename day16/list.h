#ifndef __LIST_H
#define __LIST_H


typedef struct lNode {
    int val;
    struct lNode *next;
} lNode;

typedef int(NodeCmp)(lNode *, lNode *);

typedef struct list {
    int size;
    int priv;
    lNode *root;
    lNode *tail;
} list;

list *listNew(void);
void listAppend(list *l, int value);
lNode *listDequeue(list *l);
void listRelease(list *l);
void listPrint(list *l);
void listQsort(list *l, NodeCmp *node_cmp);
list *listUniqSort(list *l, NodeCmp *node_cmp);
int listHas(list *l, int value);
int listEQ(list *l1, list *l2);
void listHeapSort(list *l);

#endif
