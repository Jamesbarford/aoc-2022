#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"

#define toint(p) ((p)-48)
#define STREQ(s, s2) (strcmp((s), (s2)) == 0)
#define HEAP_SIZE (1 << 15)
#define DISK_SIZE (70000000UL)
#define REQUIRED_SIZE (30000000UL)

/* Heap helpers */
#define __parent(x) ((x) / 2)
#define __child(x) ((x)*2)

enum FileType {
    _DIR,
    _FILE,
};

typedef struct fNode {
    enum FileType type;
    unsigned long size;
    char *name;
    struct fNode *next;
    struct fNode *child;
    struct fNode *parent;
} fNode;

typedef struct FS {
    fNode *root;
} FS;

typedef struct hNode {
    fNode *value;
} hNode;

typedef struct minheap {
    int size;
    fNode *slab[HEAP_SIZE];
} minheap;

minheap *minHeapNew(void) {
    minheap *mh = malloc(sizeof(minheap));
    mh->size = 0;
    return mh;
}

void swap(minheap *mh, int i, int j) {
    fNode *tmp = mh->slab[i];
    mh->slab[i] = mh->slab[j];
    mh->slab[j] = tmp;
}

void heapifyUp(minheap *mh, int idx) {
    int cur = idx;

    while (cur && (mh->slab[cur] != NULL && mh->slab[__parent(cur)]) &&
            mh->slab[cur]->size < mh->slab[__parent(cur)]->size)
    {
        int parent = __parent(cur);
        swap(mh, parent, cur);
        cur = parent;
    }
}

void minheapInsert(minheap *mh, fNode *value) {
    mh->slab[++mh->size] = value;
    heapifyUp(mh, mh->size);
}

void heapifyDown(minheap *mh, int idx) {
    int cur = idx;

    while (__child(cur) <= mh->size) {
        int child = __child(cur);
        fNode *c1 = mh->slab[child];
        fNode *c2 = mh->slab[child + 1];
        fNode *c = mh->slab[cur];

        if (c1 && c2) {
            if (child < mh->size && c2->size < c1->size) {
                child++;
            }
        }

        c1 = mh->slab[child];

        if (c->size <= c1->size) {
            return;
        }

        swap(mh, cur, child);
        cur = child;
    }
}

fNode *minheapLow(minheap *mh) {
    if (mh->size == 0) {
        return NULL;
    }

    return mh->slab[1];
}

void minheapRemove(minheap *mh) {
    if (mh->size == 0) {
        return;
    }

    swap(mh, mh->size, 0);
    mh->size--;
    heapifyDown(mh, 0);
}

void minheapRelease(minheap *mh) {
    if (mh) {
        free(mh);
    }
}

typedef struct lNode {
    void *value;
    struct lNode *next;
} lNode;

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

void listAppend(list *l, void *value) {
    lNode *ln = malloc(sizeof(lNode));
    ln->value = value;
    ln->next = NULL;

    if (l->root == NULL) {
        l->root = l->tail = ln;
    } else {
        l->tail->next = ln;
        l->tail = ln;
    }

    l->size++;
}

void *listDequeue(list *l) {
    if (l->size == 0) {
        return NULL;
    }

    lNode *rem = l->root;
    void *value = rem->value;
    l->root = rem->next;
    free(rem);
    l->size--;
    return value;
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

typedef struct sNode {
    void *value;
    struct sNode *next;
} sNode;

typedef struct stack {
    int size;
    sNode *root;
    sNode *tail;
} stack;

stack *stackNew(void) {
    stack *s = malloc(sizeof(stack));
    s->root = s->tail = NULL;
    s->size = 0;
    return s;
}

void stackPush(stack *s, void *value) {
    sNode *sn = malloc(sizeof(sNode));
    sn->value = value;
    sn->next = s->root;
    s->root = sn;
    s->size++;
}

void *stackPop(stack *s) {
    if (s->size == 0) {
        return NULL;
    }

    sNode *rem = s->root;
    void *value = rem->value;
    s->root = rem->next;
    free(rem);
    s->size--;

    return value;
}

void stackRelease(stack *s) {
    if (s) {
        sNode *sn = s->root;
        sNode *next = NULL;
        while (sn) {
            next = sn->next;
            free(sn);
            sn = next;
        }
        free(s);
    }
}

int advanceTill(char *ptr, char dest) {
    int offset = 0;
    while (*ptr != dest) {
        ++(ptr);
        ++offset;
    }
    return offset;
}

int collectUntill(char *ptr, char *outptr, char terminator) {
    int offset = 0;
    while (*ptr != terminator) {
        outptr[offset] = *ptr;
        ++ptr;
        ++offset;
    }
    outptr[offset] = '\0';
    return offset;
}

int matchNext(char *ptr, char needle) {
    return *(ptr + 1) == needle;
}

fNode *fNodeNew(enum FileType type) {
    fNode *fn = malloc(sizeof(fNode));
    fn->next = NULL;
    fn->child = NULL;
    fn->parent = NULL;
    fn->name = NULL;
    fn->size = 0;
    fn->type = type;
    return fn;
}

FS *FSNew(void) {
    FS *fs = malloc(sizeof(FS));
    fs->root = fNodeNew(_DIR);
    fs->root->name = "/";
    return fs;
}

void fNodePrint(fNode *fn, int depth) {
    while (depth) {
        printf(" ");
        --depth;
    }
    if (fn->type == _DIR) {
        printf("%s (dir) %lu\n", fn->name, fn->size);
    } else {
        printf("%s (file, size=%lu)\n", fn->name, fn->size);
    }
}

void __FSPrint(fNode *fn, int depth) {
    while (fn) {
        fNodePrint(fn, depth);

        if (fn->child != NULL) {
            __FSPrint(fn->child, depth + 1);
        }

        fn = fn->next;
    }
}

void FSPrint(FS *fs) {
    __FSPrint(fs->root, 0);
}

int parseName(char *ptr, fNode *fn) {
    char tmp[BUFSIZ] = {0};
    int i = 0;
    while (*ptr != '\n') {
        tmp[i] = *ptr;
        ++ptr;
        ++i;
    }

    fn->name = malloc(sizeof(char) * i + 1);
    int written = snprintf(fn->name, i + 1, "%s", tmp);
    fn->name[written] = '\0';
    return i;
}

int parseDir(char *ptr, fNode *fn) {
    int i = 0;
    ptr += advanceTill(ptr, ' ');
    ptr++;
    fn->type = _DIR;
    i += parseName(ptr, fn);
    return i;
}

int parseFile(char *ptr, fNode *fn) {
    int i = 0;
    fn->type = _FILE;

    while (isdigit(*ptr)) {
        fn->size = fn->size * 10 + toint(*ptr);
        ++i;
        ++ptr;
    }

    i++;
    ptr++;

    i += parseName(ptr, fn);
    return i;
}

int parseOutputLine(fNode *fn, char *ptr) {
    if (fn->type == _FILE) {
        return parseFile(ptr, fn);
    }
    return parseDir(ptr, fn);
}

int parseTerminalOutput(fNode *root, char *ptr) {
    int offset = 0;
    fNode *cur = NULL;

    while (*ptr != '$' && *ptr != '\0') {
        fNode *fn = NULL;
        if (*ptr == 'd' && *(ptr + 1) == 'i') {
            fn = fNodeNew(_DIR);
            int next = parseOutputLine(fn, ptr);
            offset += next;
            ptr += next;
        } else if (isdigit(*ptr)) {
            fn = fNodeNew(_FILE);
            int next = parseOutputLine(fn, ptr);
            offset += next;
            ptr += next;
        }

        if (fn) {
            fn->parent = root;
        }

        if (cur == NULL && fn) {
            cur = fn;
            root->child = fn;
        } else if (fn) {
            cur->next = fn;
            cur = fn;
        }

        int next = advanceTill(ptr, '\n');
        ptr += next + 1;
        offset += next + 1;
    }

    cur = root->child;

    return offset - 1;
}

/* pass in fn in case it is relative to the current node with '..'*/
fNode *FSChangeDirectory(fNode *fn, char *dirname) {
    /* Going up a dir */
    if (dirname[0] == '.' && dirname[1] == '.') {
        return fn->parent;
    }

    /* Reset position to start of the list */
    fn = fn->child;

    while (fn) {
        if (fn->type == _DIR) {
            if (STREQ(fn->name, dirname)) {
                return fn;
            }
        }

        fn = fn->next;
    }

    return fn;
}

void parseInstructions(FS *fs, char *instructions) {
    char tmp[BUFSIZ] = {0};
    char *ptr = instructions;
    fNode *root = fs->root;

    ptr += advanceTill(ptr, '\n');
    ptr++;

    while (*ptr) {
        if (*ptr == '$') {
            ptr += advanceTill(ptr, ' ');
            ptr++;
            switch (*ptr) {
                case 'l':
                    if (matchNext(ptr, 's')) {
                        ptr += advanceTill(ptr, 's');
                        ptr++;
                        ptr += parseTerminalOutput(root, ptr);
                    }
                    break;

                case 'c':
                    if (matchNext(ptr, 'd')) {
                        ptr += advanceTill(ptr, ' ');
                        ptr++;
                        ptr += collectUntill(ptr, tmp, '\n');
                        root = FSChangeDirectory(root, tmp);
                    }
                    break;
            }
        }
        ++ptr;
    }
}

static void __FSSetDirectorySizes(fNode *dir, fNode *fn) {
    while (fn) {
        if (fn->type == _FILE) {
            dir->size += fn->size;
        } else {
            __FSSetDirectorySizes(fn, fn->child);
            dir->size += fn->size;
        }
        fn = fn->next;
    }
}

void FSSetDirectorySizes(FS *fs) {
    __FSSetDirectorySizes(fs->root, fs->root->child);
}

/* Fake recursion, easier to read and see what is going on,
 * min heap maintains weak ordering
 * */
minheap *decendingOrderDirs(fNode *fn) {
    stack *s = stackNew();
    minheap *dirs = minHeapNew();
    stackPush(s, fn);

    while ((fn = stackPop(s)) != NULL) {
        if (fn->type == _DIR) {
            minheapInsert(dirs, fn);
        }

        if (fn->child != NULL) {
            stackPush(s, fn->child);
        }

        if (fn->next != NULL) {
            stackPush(s, fn->next);
        }
    }

    stackRelease(s);

    return dirs;
}

unsigned long solve(rFile *rf) {
    FS *fs = FSNew();
    parseInstructions(fs, rf->buf);
    FSSetDirectorySizes(fs);
    minheap *mh = decendingOrderDirs(fs->root);
    unsigned long unused = DISK_SIZE - fs->root->size;
    unsigned long required = REQUIRED_SIZE - unused;
    unsigned long freed = 0;

    printf("TOTAL: %lu\nUNUSED: %lu\nREQUIRED: %lu\nREQ: %lu\n", DISK_SIZE,
            unused, required, REQUIRED_SIZE);

    fNode *fn;
    while (mh->size > 0) {
        fn = minheapLow(mh);

        if (fn) {
            if (fn->size >= required) {
                freed = fn->size;
                break;
            }
        }
        minheapRemove(mh);
    }

    minheapRelease(mh);
    return freed;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    unsigned long solution = solve(rf);
    printf("solution = %lu\n", solution);

    rFileRelease(rf);
}
