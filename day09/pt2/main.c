#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define X (0)
#define Y (1)
#define SET_SIZE (1 << 12)

typedef struct lNode {
    long coord[2];
    struct lNode *next;
} lNode;

typedef struct list {
    int size;
    lNode *head;
    lNode *tail;
} list;

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->head = l->tail = NULL;
    l->size = 0;
    return l;
}

void listPopHead(list *r) {
    if (r->size == 0) {
        return;
    }
    lNode *head = r->head;
    r->head = head->next;
    free(head);
    r->size--;
}

void listAppend(list *r, long coord[2]) {
    lNode *n = malloc(sizeof(lNode));
    n->coord[X] = coord[X];
    n->coord[Y] = coord[Y];
    n->next = NULL;

    if (r->size == 0) {
        r->tail = r->head = n;
    } else {
        r->tail->next = n;
        r->tail = n;
    }
    r->size++;
}

void listRelease(list *r) {
    if (r) {
        lNode *n = r->head;
        lNode *next = NULL;
        while (n && r->size) {
            next = n->next;
            free(n);
            n = next;
            r->size--;
        }
        free(r);
    }
}

typedef struct sNode {
    long coord[2];
    struct sNode *next;
} sNode;

typedef struct set {
    int size;
    sNode *data[SET_SIZE];
} set;

/* really really bad */
unsigned long hashfunc(long coord[2]) {
    return (unsigned long)(coord[X] + coord[Y]) & (SET_SIZE - 1);
}

int setGetIdx(set *s, long coord[2], int *has) {
    unsigned long hash = hashfunc(coord);
    sNode *sn = s->data[hash];

    *has = 0;

    while (sn) {
        if (sn->coord[X] == coord[X] && sn->coord[Y] == coord[Y]) {
            *has = 1;
            goto out;
        }
        sn = sn->next;
    }
out:
    return hash;
}

void setAdd(set *s, long coord[2]) {
    int has = 0;
    unsigned long hash = setGetIdx(s, coord, &has);
    if (has) {
        return;
    }
    sNode *sn = malloc(sizeof(sNode));
    sn->coord[X] = coord[X];
    sn->coord[Y] = coord[Y];
    sn->next = s->data[hash];
    s->data[hash] = sn;
    s->size++;
}

set *setNew(void) {
    set *s = malloc(sizeof(set));
    s->size = 0;
    return s;
}

void setRelease(set *s) {
    if (s) {
        for (int i = 0; i < SET_SIZE && s->size; ++i) {
            sNode *sn = s->data[i];
            sNode *next = NULL;
            while (sn) {
                next = sn->next;
                free(sn);
                sn = next;
                s->size--;
            }
            s->data[i] = NULL;
        }
        free(s);
    }
}

void setPrint(set *s) {
    printf("{");
    for (int i = 0; i < SET_SIZE; ++i) {
        sNode *sn = s->data[i];
        while (sn) {
            printf("(%ld, %ld)\n", sn->coord[X], sn->coord[Y]);
            sn = sn->next;
        }
    }
    printf("}\n");
}

void moveCoord(long coord[2], char dir) {
    switch (dir) {
        case 'U':
            printf("coord[Y] = %ld\n", coord[Y]);
            --coord[Y];
            printf("coord[Y] = %ld\n", coord[Y]);
            break;

        case 'D':
            ++coord[Y];
            break;

        case 'L':
            --coord[X];
            break;

        case 'R':
            ++coord[X];
            break;

        default:
            fprintf(stderr, "Invalid direction: %c\n", dir);
            exit(EXIT_FAILURE);
    }
}

void moveTo(long coord[2], long x, long y) {
    coord[X] = x;
    coord[Y] = y;
}

int istouch(long head[2], long tail[2]) {
    if (head[X] == tail[X] && head[Y] + 1 == tail[Y]) {
        return 1;
    } else if (head[X] == tail[X] && head[Y] - 1 == tail[Y]) {
        return 1;
    } else if (head[X] + 1 == tail[X] && head[Y] == tail[Y]) {
        return 1;
    } else if (head[X] - 1 == tail[X] && head[Y] == tail[Y]) {
        return 1;
    } else if (head[X] - 1 == tail[X] && head[Y] - 1 == tail[Y]) {
        return 1;
    } else if (head[X] - 1 == tail[X] && head[Y] + 1 == tail[Y]) {
        return 1;
    } else if (head[X] + 1 == tail[X] && head[Y] + 1 == tail[Y]) {
        return 1;
    } else if (head[X] + 1 == tail[X] && head[Y] - 1 == tail[Y]) {
        return 1;
    } else if (head[X] == tail[X] && head[Y] == tail[Y]) {
        return 1;
    }
    return 0;
}

void printNode(lNode *ln) {
    printf("(%ld, %ld) ", ln->coord[X], ln->coord[Y]);
}

long signum(long val) {
    if (val == 0) {
        return 0;
    } else if (val < 0) {
        return -1;
    } else {
        return 1;
    }
}

void differenceToMove(long diff[2], long to_move[2]) {
    long xabs = labs(diff[X]);
    long yabs = labs(diff[Y]);

    /* touching */
    if ((xabs == 1 && yabs == 1) || (xabs == 1 && yabs == 0) ||
            (yabs == 1 && xabs == 0))
    {
        to_move[X] = 0;
        to_move[Y] = 0;
    } else if (diff[Y] == 0 && xabs > 1) {
        to_move[X] = diff[X] - signum(diff[X]);
        to_move[Y] = 0;
    } else if (diff[X] == 0 && yabs > 1) {
        to_move[X] = 0;
        to_move[Y] = diff[Y] - signum(diff[Y]);
    } else if (diff[X] == 0 && diff[Y] == 0) {
        to_move[X] = 0;
        to_move[Y] = 0;
    } else if ((xabs == 1 && yabs == 2) || (yabs == 1 && xabs == 2) ||
               (xabs == 2 && yabs == 2))
    {
        to_move[X] = signum(diff[X]);
        to_move[Y] = signum(diff[Y]);
    }
}

void listMove(list *l, set *seen, long prev_head[2]) {
    lNode *parent = l->head;
    lNode *child = parent->next;
    long pcoord[2] = {prev_head[X], prev_head[Y]};
    long to_move[2] = {0, 0};

    while (child) {
        if (istouch(parent->coord, child->coord)) {
            break;
        }

        long xdiff = parent->coord[X] - child->coord[X];
        long ydiff = parent->coord[Y] - child->coord[Y];
        pcoord[X] = xdiff;
        pcoord[Y] = ydiff;

        differenceToMove(pcoord, to_move);

        child->coord[X] += to_move[X];
        child->coord[Y] += to_move[Y];

        parent = child;
        child = child->next;
    }
    setAdd(seen, l->tail->coord);
}

void move(set *seen, list *l, char dir, long motion) {
    long prev_head[2] = {0, 0};

    for (long i = 0; i < motion; ++i) {
        moveCoord(l->head->coord, dir);
        listMove(l, seen, prev_head);
    }
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[10] = {0};
    long start[2] = {0, 0};
    set *seen = setNew();
    list *l = listNew();

    for (int i = 0; i < 10; ++i) {
        listAppend(l, start);
    }

    setAdd(seen, start);

    while (fgets(buf, sizeof(buf), fp)) {
        char dir = buf[0];
        long motion = strtoll(&buf[2], NULL, 10);

        printf("== %c %ld ==\n", dir, motion);
        move(seen, l, dir, motion);
        printf("--\n");
    }

    fclose(fp);
    printf("visited: %d\n", seen->size);
    setRelease(seen);

    listRelease(l);
    return 0;
}
