#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BOUNDS 4000000

int dir[][2] = {
        {-1, -1},
        {-1, 1},
        {1, 1},
        {1, -1},
};

typedef struct sensor {
    int x, y;
    int bx, by;
    int mdistance;
} sensor;

typedef struct lNode {
    int v1, v2;
    struct lNode *next;
} lNode;

typedef int(NodeCmp)(lNode *, lNode *);

/* FIFO QUEUE */
typedef struct list {
    int size;
    lNode *root;
    lNode *tail;
} list;

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->root = l->tail = NULL;
    l->size = 0;
    return l;
}

void listAppend(list *l, int start, int end) {
    lNode *ln = malloc(sizeof(lNode));
    ln->v1 = start;
    ln->v2 = end;
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
        printf("(%d, %d)", ln->v1, ln->v2);
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
    return ((n1->v1 > n2->v1) || ((n1->v1 == n2->v1) && (n1->v2 > n2->v2)));
}

/* ASC: From low to high */
int __lnode_cmp_GTE(lNode *n1, lNode *n2) {
    return ((n1->v1 < n2->v1) || ((n1->v1 == n2->v1) && (n1->v2 < n2->v2)));
}

int __lnode_cmp_start_LTE(lNode *n1, lNode *n2) {
    return n1->v1 <= n2->v1;
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

            __swap(&head->v1, &cursor->v1);
            __swap(&head->v2, &cursor->v2);

            head = head->next;
        }
        cursor = cursor->next;
    }

    __swap(&head->v1, &tail->v1);
    __swap(&head->v2, &tail->v2);

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
        if (prev->v1 != next->v1 && prev->v2 != next->v2) {
            listAppend(uniq, prev->v1, prev->v2);
        }
        prev = next;
        next = next->next;
    }

    if (uniq->tail->v1 != prev->v1 && uniq->tail->v2 != prev->v2) {
        listAppend(uniq, prev->v1, prev->v2);
    }
    // listQsort(uniq, __lnode_cmp_GTE);

    return uniq;
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
        int start = ln1->v1;
        int end = ln1->v2;
        int next_start = ln2->v1;
        int next_end = ln2->v2;

        if (start >= next_start && end <= next_end) {
            return 1;
        } else if (start <= next_start && end >= next_end) {
            return 1;
        } else if (start <= next_end && end >= next_end) {
            return 1;
        } else if (start <= next_start && end >= next_start) {
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
    listAppend(aux, ln->v1, ln->v2);

    while (ln) {
        an = aux->root;

        // [(-189975, 5137544), (-83453, 1291977)]
        while (an) {
            int start = ln->v1;
            int end = ln->v2;
            int cmp_start = an->v1;
            int cmp_end = an->v2;
            /* Fully contained */
            if (start == end) {
                break;
            }

            /* Node is fully contained within current range, null it out*/
            if (start > cmp_start && end < cmp_end) {
                ln->v1 = end;
                break;
            }

            /* Expand `an` as it can absorb `ln`, null out ln */
            if (ln->v1 <= an->v1 && ln->v2 >= an->v2) {
                an->v1 = ln->v1;
                an->v2 = ln->v2;
                ln->v1 = ln->v2;
                break;
            }

            /* If the end of the current node encroaches, then it's end = start
             * - 1 */
            if (end >= cmp_start && end <= cmp_end) {
                ln->v2 -= an->v1 - 1;
            }

            /* start `ln` overlaps end of `an` then it's start = end + 1 */
            if (start <= cmp_end && end > cmp_end) {
                ln->v1 = an->v2 + 1;
            }

            an = an->next;
        }

        if (ln->v1 != ln->v2) {
            listAppend(aux, ln->v1, ln->v2);
        }

        ln = ln->next;
    }
    listPrint(aux);
    listRelease(l);
    return aux;
}

int canfind(sensor *sensors, int sensor_count, int x, int y) {
    int can_find = 0;
    for (int i = 0; i < sensor_count; ++i) {
        sensor *s = sensors + i;
        if (s->bx == x && s->by == y) {
            return 0;
        }

        if (manhattanDistance(x, s->x, y, s->y) <= s->mdistance) {
            can_find = 1;
            continue;
        }
    }

    return can_find;
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
        for (int i = ln->v1; i <= ln->v2; ++i) {
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

    if (file_name[0] == '.' && file_name[1] == '/') {
        file_name += 2;
    }

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    char tmp[BUFSIZ];
    int row_of_interest =
            (strncasecmp(file_name, "input.txt", 5) == 0) ? 2000000 : 10;
    list *l = listNew();
    list *beacons = listNew();
    int scount = 0;
    sensor *sensors = malloc(sizeof(sensor) * 1000);

    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t len = strlen(tmp);
        char *ptr = tmp;
        if (tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
            --len;
        }

        sensor *s = &sensors[scount];

        int x, y, x2, y2;
        sscanf(tmp, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d", &x,
                &y, &x2, &y2);
        int m_distance = manhattanDistance(x, x2, y, y2);
        s->x = x;
        s->y = y;
        s->bx = x2;
        s->by = y2;
        s->mdistance = m_distance;
        ++scount;

        int shift = calculateXShift(y, m_distance, row_of_interest);
        int S = x - shift;
        int F = x + shift;

        /* for part 1*/
        int inrange = sensorInRange(y, row_of_interest, m_distance);
        if (inrange) {
            printf("S=%3d <> F=%3d sensor=(%3d, %3d) manhattan distance=%3d\n",
                    S, F, x, y, m_distance);
            listAppend(l, S, F);
        }

        listAppend(beacons, x2, y2);
    }
    fclose(fp);
    printf("sensor count: %d\n", scount);

    listQsort(beacons, __lnode_cmp_GTE);
    list *uniq_beacons = listUniqSort(beacons);

    int beacon_count = 0;
    for (lNode *ln = uniq_beacons->root; ln != NULL; ln = ln->next) {
        /* end == y */
        if (ln->v2 == row_of_interest) {
            beacon_count++;
        }
    }

    printf("part1: %d\n", solution1(l, beacon_count));

    for (int i = 0; i < scount; ++i) {
        sensor *s = &sensors[i];
        int x = s->x;
        int y = s->y;
        int mdist = s->mdistance+1;
        x += mdist;

        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < mdist; k++) {
                if (!canfind(sensors,scount,x,y) &&
                        x >= 0 && x <= MAX_BOUNDS &&
                        y >= 0 && y <= MAX_BOUNDS)
                {
                    printf("(%d, %d)\n", x, y);
                    printf("part2: %ld\n", (long)MAX_BOUNDS * x + y);
                    return 0;
                }
                if (k != mdist - 1) {
                    x += dir[j][0];
                    y += dir[j][1];
                }
            }
        }
    }
    listRelease(uniq_beacons);
    listRelease(beacons);
    listRelease(l);
    free(sensors);

    return 0;
}
