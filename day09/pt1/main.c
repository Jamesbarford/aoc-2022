#include <stdio.h>
#include <stdlib.h>

#define X (0)
#define Y (1)
#define SET_SIZE (1 << 12)

typedef struct sNode {
    long coord[2];
    struct sNode *next;
} sNode;

typedef struct set {
    int size;
    sNode *data[SET_SIZE];
} set;

/* really really bad */
long hashfunc(long coord[2]) {
    return (coord[X] + coord[Y]) & (SET_SIZE - 1);
}

int setGetIdx(set *s, long coord[2], int *has) {
    long hash = hashfunc(coord);
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
    long hash = setGetIdx(s, coord, &has);
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

void moveCoord(long coord[2], char dir) {
    switch (dir) {
        case 'U':
            --coord[Y];
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

void move(set *seen, char dir, int motion, long head[2],
        long tail[2])
{
    long prev_head[2] = {0, 0};

    for (int i = 0; i < motion; ++i) {
        if (i != 0 && !istouch(head, tail)) {
            setAdd(seen, prev_head);
            moveTo(tail, prev_head[X], prev_head[Y]);
        }

        prev_head[X] = head[X];
        prev_head[Y] = head[Y];
        moveCoord(head, dir);
    }

    if (!istouch(head, tail)) {
        setAdd(seen, prev_head);
        moveTo(tail, prev_head[X], prev_head[Y]);
    }
    printf("head: (%ld, %ld)\n", head[X], head[Y]);
    printf("tail: (%ld, %ld)\n", tail[X], tail[Y]);
}

int main(void) {
    FILE *fp = fopen("./warmup.txt", "r");
    char buf[10] = {0};
    long head[2] = {0, 0};
    long tail[2] = {0, 0};
    set *seen = setNew();

    setAdd(seen, tail);

    while (fgets(buf, sizeof(buf), fp)) {
        char dir = buf[0];
        int motion = (int)strtoll(&buf[2], NULL, 10);

        printf("== %c %d ==\n", dir, motion);
        move(seen, dir, motion, head, tail);
        printf("--\n");
    }

    fclose(fp);
    printf("visited: %d\n", seen->size);
    setRelease(seen);
    return 0;
}
