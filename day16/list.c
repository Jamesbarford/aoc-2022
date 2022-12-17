#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "list.h"

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->root = l->tail = NULL;
    l->size = 0;
    return l;
}

void listAppend(list *l, int value) {
    lNode *ln = malloc(sizeof(lNode));
    ln->val = value;
    ln->next = NULL;

    if (l->root == NULL) {
        l->root = l->tail = ln;
    } else {
        l->tail->next = ln;
        l->tail = ln;
    }

    l->size++;
}

lNode *listDequeue(list *l) {
    if (l->size == 0) {
        return NULL;
    }

    lNode *rem = l->root;
    l->root = rem->next;
    l->size--;
    rem->next = NULL;
    return rem;
}

void listRelease(list *l) {
    if (l) {
        lNode *ln = l->root;
        lNode *next = NULL;
        while (ln) {
            next = ln->next;
            free(ln);
            ln = next;
        }
        free(l);
    }
}

void listPrint(list *l) {
    int len = l->size;
    lNode *ln = l->root;
    printf("[");
    while (ln && len) {
        printf("%d", ln->val);
        if (len - 1 != 0) {
            printf(", ");
        }

        --len;
        ln = ln->next;
    }
    printf("]\n");
}


/* DESC: From high to low 
static int __lnode_cmp_LTE(lNode *n1, lNode *n2) {
    return (n1->val >= n2->val);
}
*/

/* ASC: From low to high */
static int __lnode_cmp_GTE(lNode *n1, lNode *n2) {
    return (n1->val <= n2->val);
}

static void __swap(int *x, int *y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

static lNode *__list_pivot(lNode *head, lNode *tail, NodeCmp *node_cmp) {
    lNode *pivot = head;
    lNode *cursor = head;

    while (cursor != NULL && cursor != tail) {
        if (node_cmp(cursor, tail)) {
            pivot = head;

            __swap(&head->val, &cursor->val);

            head = head->next;
        }
        cursor = cursor->next;
    }

    __swap(&head->val, &tail->val);

    return pivot;
}

static void __list_qsort(lNode *head, lNode *tail, NodeCmp *node_cmp) {
    if (head == tail) {
        return;
    }
    lNode *pivot = __list_pivot(head, tail, node_cmp);

    if (pivot != NULL && pivot->next != NULL) {
        __list_qsort(pivot->next, tail, node_cmp);
    }

    if (pivot != NULL && head != pivot) {
        __list_qsort(head, pivot, node_cmp);
    }
}

void listQsort(list *l, NodeCmp *node_cmp) {
    __list_qsort(l->root, l->tail, node_cmp);
}

void __intswap(int *i, int *j) {
    int tmp = *i;
    *i = *j;
    *j = tmp;
}

void listHeapSort(list *l) {
    int *heap = malloc(l->size*sizeof(int));
    for (int i = 0; i < l->size;++i) {
        heap[i] = INT_MAX;
    }
    int heap_size = 0;
    lNode *ln = NULL;

    /* Fill the heap */
    while ((ln = listDequeue(l))) {
        heap[++heap_size] = ln->val;
        int cur = heap_size;
        while (cur && heap[cur/2] > heap[cur]) {
            int parent = cur/2;
            __intswap(&heap[parent], &heap[cur]);
            cur = parent;
        }
        free(ln);
    }

    /* Fill list */
    while (heap_size > 0) {
        int min = heap[0];
        listAppend(l, min);
        __intswap(&heap[0], &heap[heap_size]);
        --heap_size;
        int cur = 0;
        while (cur*2 <= heap_size) {
            int child = cur * 2;
            if (child < heap_size && heap[child+1] < heap[child]) {
                ++child;
            }

            if (heap[cur] <= heap[child]) {
                break;
            }
            __intswap(&heap[child], &heap[cur]);
            cur = child;
        }
    }
    free(heap);
}

/* Really really slow, output is sorted and unique */
list *listUniqSort(list *l, NodeCmp *node_cmp) {
    listQsort(l, node_cmp);
    list *uniq = listNew();
    if (l->size == 0) {
        return uniq;
    }

    lNode *prev = l->root;
    lNode *next = prev->next;

    if (!next) {
        listAppend(l, prev->val);
    }
    return l;

    while (next) {
        if (prev->val != next->val) {
            listAppend(uniq, prev->val);
        }
        prev = next;
        next = next->next;
    }

    if (uniq->tail->val != prev->val) {
        listAppend(uniq, prev->val);
    }
    listQsort(uniq, node_cmp);
    return uniq;
}

int listHas(list *l, int value) {
    for (lNode *ln = l->root; ln != NULL; ln = ln->next) {
        if (ln->val == value) {
            return 1;
        }
    }
    return 0;
}

int listEQ(list *l1, list *l2) {
    if (l1->size != l2->size) {
        return 0;
    }
    listQsort(l1, __lnode_cmp_GTE);
    listQsort(l2, __lnode_cmp_GTE);

    /* Can't rely on size as I use that to store the pressure */
    lNode *n1 = l1->root;
    lNode *n2 = l2->root;

    while (n1 && n2) {
        if (n1->val != n2->val) {
            return 0;
        }

        n1 = n1->next;
        n2 = n2->next;
    }

    return n1 == NULL && n2 == NULL;
}
