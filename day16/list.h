#ifndef __LIST_H
#define __LIST_H


typedef struct lNode {
    long val;
    struct lNode *next;
} lNode;

typedef int(NodeCmp)(lNode *, lNode *);

typedef struct list {
    long size;
    long priv;
    lNode *root;
    lNode *tail;
} list;

list *listNew(void);
void listAppend(list *l, long value);
lNode *listDequeue(list *l);
void listRelease(list *l);
void listQsort(list *l, NodeCmp *node_cmp);
list *listCopy(list *l);
list *listUniqSort(list *l, NodeCmp *node_cmp);
int  listHas(list *l, long value);
int listEQ(list *l1, list *l2);
void listHeapSort(list *l);
void listPrint(list *l);
list **listGetAllCombinations(list *l, int combo_size, int rounds, int *actual);
void listRemove(list *l, long val);

static int __lnode_cmp_LTE(lNode *n1, lNode *n2) {
    return (n1->val >= n2->val);
}

/* ASC: From low to high */
static int __lnode_cmp_GTE(lNode *n1, lNode *n2) {
    return (n1->val <= n2->val);
}

#endif
