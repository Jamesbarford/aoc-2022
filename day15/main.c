#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct lNode {
    int start, end;
    struct lNode *next;
} lNode;

/* FIFO QUEUE */
typedef struct list {
    int size;
    lNode *root;
    lNode *tail;
} list;

typedef int(NodeCmp)(lNode *, lNode *);

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->root = l->tail = NULL;
    l->size = 0;
    return l;
}

void listAppend(list *l, int start, int end) {
    lNode *ln = malloc(sizeof(lNode));
    ln->start = start;
    ln->end = end;
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
        printf("(%d, %d)", ln->start, ln->end);
        if (len - 1 != 0) {
            printf(", ");
        }

        --len;
        ln = ln->next;
    }
    printf("]\n");
}


/* DESC: From high to low */
int __lnode_cmp_LTE(lNode *n1, lNode *n2) {
    return ((n1->start > n2->start) ||
            ((n1->start == n2->start) && (n1->end > n2->end)));
}

/* ASC: From low to high */
int __lnode_cmp_GTE(lNode *n1, lNode *n2) {
    return ((n1->start < n2->start) ||
            ((n1->start == n2->start) && (n1->end < n2->end)));
}

int __lnode_cmp_start_LTE(lNode *n1, lNode *n2) {
    return n1->start <= n2->start;
}

void __swap(int *x, int *y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

lNode *__list_pivot(lNode *head, lNode *tail, NodeCmp *node_cmp) {
    lNode *pivot = head;
    lNode *cursor = head;

    while (cursor != NULL && cursor != tail) {
        if (node_cmp(cursor, tail)) {
            pivot = head;

            __swap(&head->start, &cursor->start);
            __swap(&head->end, &cursor->end);

            head = head->next;
        }
        cursor = cursor->next;
    }

    __swap(&head->start, &tail->start);
    __swap(&head->end, &tail->end);

    return pivot;
}

void __list_qsort(lNode *head, lNode *tail, NodeCmp *node_cmp) {
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

/* Really really slow, output is sorted and unique */
list *listUniqSort(list *l) {
    listQsort(l, __lnode_cmp_GTE);
    list *uniq = listNew();
    lNode *prev = l->root;
    lNode *next = prev->next;

    while (next) {
        if (prev->start != next->start && prev->end != next->end) {
            listAppend(uniq,prev->start,prev->end);
        }
        prev = next;
        next = next->next;
    }

    if (uniq->tail->start != prev->start && uniq->tail->end != prev->end) {
        listAppend(uniq,prev->start,prev->end);
    }
    listQsort(uniq,__lnode_cmp_GTE);

    return uniq;
}

int munchChar(char *ptr, char until) {
    int offset = 0;
    while (*ptr != until) {
        ++ptr;
        ++offset;
    }
    return offset;
}

int getNumber(char *ptr, int *output) {
    int offset = 0;
    int tmp_num = 0;
    int is_minus = 0;
    if (*ptr == '-') {
        is_minus = 1;
        ptr++;
    }

    while (isdigit(*ptr)) {
        tmp_num = tmp_num * 10 + *ptr - 48;
        ++ptr;
        ++offset;
    }

    tmp_num = is_minus ? -1 * tmp_num : tmp_num;
    *output = tmp_num;
    return offset;
}

/*
 *
 *
 *
 Manhattan distance
 (x=8,y=7)
 d_m = abs(x_b - x_s) + abs(y_b - y_s)  -> 9


 y_diff = y_s - R
 x_shift = (d_m - y_diff)
 S = x_s - x_shift
 F = x_s + x_shift
 * */

int calculateXShift(int y, int m_distance, int R) {
    int y_diff = abs(R - y);
    int x_shift = (m_distance - y_diff);
    return x_shift;
}

int manhattanDistance(int x, int x2, int y, int y2) {
    return abs(x - x2) + abs(y - y2);
}

int sensorInRange(int y, int row, int m_distance) {
    return row >= y - m_distance && row <= y + m_distance;
}

int listHasOverlap(list *l) {
    listQsort(l, __lnode_cmp_GTE);
    lNode *ln1 = l->root;
    lNode *ln2 = ln1->next;

    while (ln1 && ln2) {
        if (ln1->start >= ln2->start && ln1->end <= ln2->end) {
            return 1;
        } else if (ln1->start <= ln2->start && ln1->end >= ln2->end) {
            return 1;
        } else if (ln1->start <= ln2->end && ln1->end >= ln2->end) {
            return 1;
        } else if (ln1->start <= ln2->start && ln1->end >= ln2->start) {
            return 1;
        }
        ln2 = ln1;
        ln1 = ln1->next;
    }

    return 0;
}

/* Frees list passed in and returns a new non-overlapping list */
list *listConsolidate(list *l) {
    lNode *ln = l->root;
    list *aux = listNew();
    lNode *an;
    listQsort(l, __lnode_cmp_GTE);
    listAppend(aux, ln->start, ln->end);

    while (ln) {
        an = aux->root;

        // [(-189975, 5137544), (-83453, 1291977)]
        while (an) {
            /* Fully contained */
            if (ln->start == ln->end) {
                break;
            }

            /* Node is fully contained within current range, null it out*/
            if (ln->start > an->start && ln->end < an->end) {
                ln->start = ln->end;
                break;
            }

            /* Expand `an` as it can absorb `ln`, null out ln */
            if (ln->start <= an->start && ln->end >= an->end) {
                an->start = ln->start;
                an->end = ln->end;
                ln->start = ln->end;
                break;
            }

            /* If the end of the current node encroaches, then it's end = start - 1 */
            if (ln->end >= an->start && ln->end <= an->end) {
                ln->end -= an->start - 1;
            }

            /* start `ln` overlaps end of `an` then it's start = end + 1 */
            if (ln->start <= an->end && ln->end > an->end) {
                ln->start = an->end + 1;
            }

            an = an->next;
        }

        if (ln->start != ln->end) {
            listAppend(aux, ln->start, ln->end);
        }

        ln = ln->next;
    }
    listPrint(aux);
    listRelease(l);
    return aux;
}

/*
 *
 S= 12 <> F= 12 sensor=( 12,  14) manhattan distance=  4 -> 1
 S=  2 <> F= 14 sensor=(  8,   7) manhattan distance=  9 -> 10
 S=  2 <> F=  2 sensor=(  2,   0) manhattan distance= 10 -> 0
 S= -2 <> F=  2 sensor=(  0,  11) manhattan distance=  3 -> 5
 S= 16 <> F= 24 sensor=( 20,  14) manhattan distance=  8 -> 8
 S= 14 <> F= 18 sensor=( 16,   7) manhattan distance=  5 -> 3
 *
 */
int solution1(list *l, int beacon_count) {
    list *aux = NULL;
    lNode *ln = NULL;
    int acc = 0;
    listPrint(l);

    while (listHasOverlap(l)) {
        aux = listConsolidate(l);
        l = aux;
    }

    for (ln = l->root; ln != NULL; ln = ln->next) {
        int tmp = 0;
        for (int i = ln->start; i <= ln->end; ++i) {
            tmp += 1;
        }
        acc += tmp;
    }

    return acc - beacon_count;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }
    char *file_name = argv[1];
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    char tmp[BUFSIZ];
    int row_of_interest = (strncasecmp(file_name, "input.txt", 5)  ==0) ?  2000000 : 10;
    list *l = listNew();
    list *beacons = listNew();

    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t len = strlen(tmp);
        char *ptr = tmp;
        if (tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
            --len;
        }

        /* point to first digit of x */
        ptr += 12;
        int x, y, x2, y2;
        ptr += getNumber(ptr, &x);
        ptr += munchChar(ptr, '=');
        ptr++;
        ptr += getNumber(ptr, &y);
        ptr += munchChar(ptr, '=');
        ptr++;
        ptr += getNumber(ptr, &x2);
        ptr += munchChar(ptr, '=');
        ptr++;
        ptr += getNumber(ptr, &y2);

        int m_distance = manhattanDistance(x, x2, y, y2);
        int shift = calculateXShift(y, m_distance, row_of_interest);
        int S = x - shift;
        int F = x + shift;

        int inrange = sensorInRange(y, row_of_interest, m_distance);

        if (inrange) {
            printf("S=%3d <> F=%3d sensor=(%3d, %3d) manhattan distance=%3d\n",
                    S, F, x, y, m_distance);
            listAppend(l, S, F);
            listAppend(beacons, x2, y2);
        }
        // printf("sensor: x=%d, y=%d, beacon: x=%d, y=%d\n", x, y, x2, y2);
    }

    printf("beacons: \n");
    listQsort(beacons, __lnode_cmp_GTE);
    listPrint(beacons);
    
    printf("uniq beacons: \n");
    list *uniq_beacons = listUniqSort(beacons);
    listPrint(uniq_beacons);

    int beacon_count = 0;
    for (lNode *ln = uniq_beacons->root; ln != NULL; ln = ln->next) {
        /* end == y */
        if (ln->end == row_of_interest) {
            beacon_count++;
        }
    }


    printf("=====\n");
    
    printf("starts & ends: ");
    listPrint(l);
    printf("beacon_count: %d\n", beacon_count);
    printf("part1 - alt: %d\n", solution1(l, beacon_count));
    fclose(fp);
    //printf("part1: %d\n", solve(l));
    return 0;
}
