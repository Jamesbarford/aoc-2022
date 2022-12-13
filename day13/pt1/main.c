#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define toint(p) ((p)-48)
#define MAX_LIST (1 << 12)

typedef struct lNode {
    int val, depth;
    struct lNode *next;
} lNode;

/* FIFO QUEUE */
typedef struct list {
    int size;
    int depth;
    lNode *root;
    lNode *tail;
} list;

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->root = l->tail = NULL;
    l->size = 0;
    l->depth = 0;
    return l;
}

void listAppend(list *l, int val, int depth) {
    lNode *ln = malloc(sizeof(lNode));
    ln->val = val;
    ln->depth = depth;
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

void listClear(list *l) {
    if (l) {
        lNode *ln = l->root;
        lNode *next = NULL;
        while (ln) {
            next = ln->next;
            free(ln);
            ln = next;
            l->size--;
        }
        l->root = NULL;
    }
}

void listRelease(list *l) {
    if (l) {
        listClear(l);
        free(l);
    }
}

void listPrint(list *l) {
    int len = l->size;
    lNode *ln = l->root;
    printf("len: %d, depth: %d\n", l->size, l->depth);
    printf("[");
    while (ln) {
        printf("%d", ln->val);
        if (len - 1 != 0) {
            printf(", ");
        }

        --len;
        ln = ln->next;
    }
    printf("]\n");
}

int listCmp(list **l1s, list **l2s) {
    int depth = 0;
    int retval = 1;

    for (int i = 0; i < MAX_LIST; ++i) {
        list *l1 = l1s[i];
        list *l2 = l2s[i];

        if (l1->size == 0 && l2->size == 0 && l1->depth == l2->depth) {
            continue;
        }

        if (l1->depth != l2->depth) {
            retval = 0;
            goto out;
        }

        listPrint(l1);
        listPrint(l2);

        lNode *ln1 = l1->root;
        lNode *ln2 = l2->root;

        while (ln1 && ln2) {
            if (ln1->val > ln2->val) {
                retval = 0;
                goto out;
            }

            ln2 = ln2->next;
            ln1 = ln1->next;
        }

        if (ln2 == NULL && ln1 != NULL) {
            retval = 0;
            goto out;
        }
    }

out:
    printf("retval: %d\n\n", retval);
    return retval;
}

void parseToList(list **lists, char *ptr) {
    int depth = -1;
    int tmp = 0;

    while (*ptr) {
        switch (*ptr) {
            case '[':
                depth++;
                lists[depth]->depth = depth;
                break;
            case ']':
                depth--;
                lists[depth]->depth = depth;
                break;
            default:
                if (isdigit(*ptr)) {
                    while (isdigit(*ptr)) {
                        tmp = tmp * 10 + toint(*ptr);
                        ++ptr;
                    }
                    listAppend(lists[depth], tmp, depth);
                    tmp = 0;
                }
                break;
        }
        ++ptr;
    }
}

int main(void) {
    FILE *fp = fopen("./warmup.txt", "r");
    char tmp[BUFSIZ];
    int list_idx = 0;
    list *l1[MAX_LIST];
    list *l2[MAX_LIST];

    for (int i = 0; i < MAX_LIST; ++i) {
        l1[i] = listNew();
        l2[i] = listNew();
    }

    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t len = strlen(tmp);
        if (tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
            --len;
        }

        if (len == 0) {
            listCmp(l1, l2);
            for (int i = 0; i < MAX_LIST; ++i) {
                listClear(l1[i]);
                listClear(l2[i]);
            }
            list_idx = 0;
            continue;
        }

        if (list_idx == 0) {
            parseToList(l1, tmp);
            list_idx++;
        } else {
            parseToList(l2, tmp);
        }
    }

    fclose(fp);
    return 0;
}
