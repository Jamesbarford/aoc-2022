#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/readfile.h"

#define toint(p) ((p)-48)
#define STREQ(s, s2) (strcmp((s), (s2)) == 0)
#define THREASHOLD (100000UL)

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
        sNode *cur = NULL;
        while (sn) {
            cur = sn;
            sn = sn->next;
            free(cur);
        }
        free(s);
    }
}

void stackReverse(stack *s) {
    sNode *prev = NULL;
    sNode *cur = s->root;
    sNode *next = NULL;

    while (cur) {
        next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    s->root = prev;
}

int advanceTil(char *ptr, char dest) {
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

int advanceToDigit(char *ptr) {
    int offset = 0;

    while (!isdigit(*ptr)) {
        ++ptr;
        ++offset;
    }

    return offset;
}

int matchNext(char *ptr, char needle) {
    return *(ptr + 1) == needle;
}

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

typedef struct FS {
    unsigned long total_size;
    fNode *root;
} FS;

FS *FSNew(void) {
    FS *fs = malloc(sizeof(FS));
    fs->total_size = 0;
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
    ptr += advanceTil(ptr, ' ');
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

        int next = advanceTil(ptr, '\n');
        ptr += next + 1;
        offset += next + 1;
    }

    cur = root->child;

    return offset - 1;
}

fNode *__FSFindDir(fNode *fn, list *q, char *dirname) {
    while (fn) {
        if (fn->type == _DIR) {
            if (strcmp(fn->name, dirname) == 0) {
                return fn;
            }

            fNode *tmp = NULL;
            while ((tmp = listDequeue(q)) != NULL) {
                __FSFindDir(fn, q, dirname);
            }
        }
        fn = fn->next;
    }

    return fn;
}

fNode *FSFindDir(FS *fs, char *dirname) {
    list *q = listNew();
    fNode *fn = __FSFindDir(fs->root, q, dirname);
    listRelease(q);
    return fn;
}

/* pass in fn in case it is relative to the current node with '..'*/
fNode *FSChangeDirectory(FS *fs, fNode *fn, char *dirname) {
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

    /* search from the root
    if (fn == NULL) {
        return FSFindDir(fs, dirname);
    }
    */

    return fn;
}

void parseInstructions(FS *fs, char *instructions) {
    char tmp[BUFSIZ] = {0};
    char *ptr = instructions;
    fNode *root = fs->root;

    ptr += advanceTil(ptr, '\n');
    ptr++;

    while (*ptr) {
        if (*ptr == '$') {
            ptr += advanceTil(ptr, ' ');
            ptr++;
            switch (*ptr) {
                case 'l':
                    if (matchNext(ptr, 's')) {
                        ptr += advanceTil(ptr, 's');
                        ptr++;
                        ptr += parseTerminalOutput(root, ptr);
                    }
                    break;

                case 'c':
                    if (matchNext(ptr, 'd')) {
                        ptr += advanceTil(ptr, ' ');
                        ptr++;
                        ptr += collectUntill(ptr, tmp, '\n');
                        root = FSChangeDirectory(fs, root, tmp);
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

list *FSDirectNodes(FS *fs, unsigned long threashold) {
    list *l = listNew();
    fNode *fn = fs->root->child;

    while (fn) {
        if (fn->type == _DIR && fn->size < threashold) {
            listAppend(l, fn);
        }
        fn = fn->next;
    }
    return l;
}

unsigned long sumNodes(list *l) {
    unsigned long acc = 0;

    fNode *fn = NULL;

    while ((fn = listDequeue(l)) != NULL) {
        fNodePrint(fn, 0);
        acc += fn->size;
        fn = fn->child;
        while (fn) {
            if (fn->type == _DIR) {
                listAppend(l, fn);
            }
            fn = fn->next;
        }
    }
    return acc;
}

/* Fake recursion, easier to read and see what is going on */
list *getCandidates(fNode *fn, unsigned long threashold) {
    stack *s = stackNew();
    list *dirs = listNew();
    stackPush(s, fn);

    while ((fn = stackPop(s)) != NULL) {
        if (fn->type == _DIR && fn->size < threashold) {
            listAppend(dirs, fn);
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
    unsigned long acc = 0;
    FS *fs = FSNew();
    parseInstructions(fs, rf->buf);
    FSSetDirectorySizes(fs);
    list *l = getCandidates(fs->root, THREASHOLD);
    fNode *fn = NULL;
    while ((fn = listDequeue(l)) != NULL) {
        acc += fn->size;
    }

    listRelease(l);

    return acc;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    unsigned long solution = solve(rf);
    printf("solution = %lu\n", solution);

    rFileRelease(rf);
}
