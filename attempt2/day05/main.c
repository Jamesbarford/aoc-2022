#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void panic(char *fmt, ...) {
    va_list ap;
    char buf[BUFSIZ];
    unsigned int len;

    va_start(ap, fmt);

    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    buf[len] = '\0';
    fprintf(stderr, "%s", buf);
    va_end(ap);
    exit(EXIT_FAILURE);
}

typedef struct fileBuf {
    char *buf;
    char *ptr;
    long size;
} fileBuf;

typedef struct listNode {
    struct listNode *next;
    struct listNode *prev;
    unsigned long value;
} listNode;

typedef struct list {
    struct listNode *root;
    unsigned long size;
} list;

typedef struct listIter {
    unsigned long count;
    int dir;
    listNode *ln;
} listIter;

#define LIST_FWD (0)
#define LIST_REV (1)

void listIterInit(listIter *it, list *l, int dir) {
    if (l->size == 0) {
        return;
    }
    it->count = l->size;
    it->dir = dir;
    if (dir == LIST_REV) {
        it->ln = l->root->prev;
    } else if (dir == LIST_FWD) {
        it->ln = l->root;
    } else {
        panic("invalid DIR: %d\n", dir);
    }
}

listNode *listIterNext(listIter *it) {
    if (it->count == 0) {
        return NULL;
    }

    listNode *ln = it->ln;
    if (it->dir == LIST_REV) {
        it->ln = ln->prev;
    } else {
        it->ln = ln->next;
    }
    it->count--;
    return ln;
}

listNode *listNodeNew(unsigned long instruction) {
    listNode *ln = malloc(sizeof(listNode));
    ln->next = ln->prev = NULL;
    ln->value = instruction;
    return ln;
}

list *listNew(void) {
    list *l = malloc(sizeof(list));
    l->size = 0;
    l->root = NULL;
    return l;
}

void listAddHead(list *l, listNode *ln) {
    if (l->size == 0) {
        l->root = ln;
        ln->next = ln->prev = ln;
    } else {
        listNode *tail = l->root->prev;
        listNode *head = l->root;
        ln->next = head;
        ln->prev = tail;
        tail->next = ln;
        head->prev = ln;
        l->root = ln;
    }
    l->size++;
}

void listAddTail(list *l, listNode *ln) {
    if (l->size == 0) {
        l->root = ln;
        ln->next = ln->prev = ln;
    } else {
        listNode *tail = l->root->prev;
        listNode *head = l->root;
        ln->next = head;
        ln->prev = tail;
        tail->next = ln;
        head->prev = ln;
    }
    l->size++;
}

listNode *listTakeHead(list *l) {
    if (l->size == 0) {
        return NULL;
    }
    listNode *head = l->root;
    if (l->size == 1) {
        l->root = NULL;
    } else {
        listNode *tail = l->root->prev;
        l->root = l->root->next;
        tail->next = l->root;
        l->root->prev = tail;
    }
    l->size--;
    return head;
}

listNode *listTakeTail(list *l) {
    if (l->size == 0) {
        return NULL;
    }
    listNode *tail = l->root->prev;
    if (l->size == 1) {
        l->root = NULL;
    } else {
        listNode *new_tail = tail->prev;
        new_tail->next = l->root;
        l->root->next = new_tail;
    }
    l->size--;
    return tail;
}

void listRelease(list *l) {
    if (l->size == 0) {
        free(l);
        return;
    }
    unsigned long count = l->size;
    listNode *ln = l->root, *tmp = ln;
    do {
        tmp = ln;
        ln = ln->next;
        free(tmp);
        count--;
    } while (count > 0);
    free(l);
}

int getStackCount(fileBuf *fb) {
    int count = 0;
    fb->ptr = fb->buf;
    while (!isdigit(*fb->ptr)) {
        fb->ptr++;
    }
    count++;
    fb->ptr++;
    while (*fb->ptr != '\n') {
        if (isdigit(*fb->ptr)) {
            count++;
        }
        fb->ptr++;
    }
    return count;
}

list **listParseCreates(fileBuf *fb, int stackcount) {
    list **stacks = malloc(sizeof(list *) * stackcount); 
    int count = 0, bol = 1, i = 0;
    listNode *ln;
    fb->ptr = fb->buf;

    for (int i = 0; i < stackcount; ++i) {
        stacks[i] = listNew();
    }

    fb->ptr++;
    while (1) {
        if (isalpha(*fb->ptr)) {
            ln = listNodeNew(*fb->ptr);
            listAddTail(stacks[i], ln);
        }

        if (*(fb->ptr + 2) == '\n') {
            fb->ptr += 4;
            /* We are on the stack position */
            if (isdigit(*fb->ptr)) {
                break;
            }
            i = 0;
        } else {
            fb->ptr += 4;
            i++;
        }
    }

    return stacks;
}

list *listParseInstructions(fileBuf *fb) {
    list *l = listNew();
    fb->ptr = fb->buf;
    while (*fb->ptr != 'm') {
        fb->ptr++;
    }

    unsigned short c1 = 0, c2 = 0, c3 = 0;
    unsigned long inst = 0;

    while (1) {
        fb->ptr += 5;
        while (isdigit(*fb->ptr)) {
            c1 = c1 * 10 + *fb->ptr++ - '0';   
        }
        fb->ptr += 6;
        while (isdigit(*fb->ptr)) {
            c2 = c2 * 10 + *fb->ptr++ - '0';   
        }
        fb->ptr += 4;
        while (isdigit(*fb->ptr)) {
            c3 = c3 * 10 + *fb->ptr++ - '0';   
        }

        inst = c1 | ((c2-1) << 16) | (((unsigned long)c3-1) << 32ul); 
    
        listAddTail(l, listNodeNew(inst));
        if (*(fb->ptr + 1) == '\0') {
            break;
        }
        c1 = c2 = c3 = 0;
        inst = 0;
        fb->ptr++;
    }

    return l;
}

void stacksPrint(list **stacks, int stackcount) {
    listIter it;
    listNode *ln;

    for (int i = 0; i < stackcount; ++i) {
        listIterInit(&it, stacks[i], LIST_REV);
        printf("[%d] ", i);
        while ((ln = listIterNext(&it)) != NULL) {
            printf("%c ", (char)ln->value);
        }
        printf("\n");
    }
}

void solve1(list **stacks, int stackcount, list *instructions) {
    listIter it;
    listNode *ln;
    unsigned short move,from,to;
    listIterInit(&it, instructions, LIST_FWD);
    
    while ((ln = listIterNext(&it)) != NULL) {
        move = ln->value & 0xFFFF;
        from = ln->value >> 16 & 0xFFFF;
        to = ln->value >> 32 & 0xFFFF;

        for (int i = 0; i < move; ++i) {
            listAddHead(stacks[to], listTakeHead(stacks[from]));
        }
    }

    for (int i = 0; i < stackcount; ++i) {
        if ((ln = stacks[i]->root) != NULL) {
            printf("%c\n", (char)ln->value);
        }
    }
}

void solve2(list **stacks, int stackcount, list *instructions) {
    listIter it, queit;
    listNode *ln, *popped;
    unsigned short move,from,to;
    list *stack = listNew();
    listIterInit(&it, instructions, LIST_FWD);
    
    while ((ln = listIterNext(&it)) != NULL) {
        move = ln->value & 0xFFFF;
        from = ln->value >> 16 & 0xFFFF;
        to = ln->value >> 32 & 0xFFFF;

        for (int i = 0; i < move; ++i) {
            listAddHead(stack, listTakeHead(stacks[from]));
        }

        while ((popped = listTakeHead(stack)) != NULL) {
            listAddHead(stacks[to], popped);
        }
    }

    for (int i = 0; i < stackcount; ++i) {
        if ((ln = stacks[i]->root) != NULL) {
            printf("%c\n", (char)ln->value);
        }
    }
    listRelease(stack);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        panic("Usage: %s <file>\n", argv[0]);
    }
    FILE *fp = fopen(argv[1], "r");
    fileBuf fb = {NULL,NULL,0};

    fseek(fp, 0, SEEK_END);
    fb.size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fb.buf = malloc(sizeof(char) + 1 * fb.size);
    assert(fread(fb.buf, sizeof(char), fb.size, fp) == fb.size);
    fb.buf[fb.size] = '\0';

    int stackcount = getStackCount(&fb);
    list **stacks = listParseCreates(&fb, stackcount);
    list *instructions = listParseInstructions(&fb);
    listIter it;
    listNode *ln;

    solve2(stacks,stackcount,instructions);

    for (int i = 0; i < stackcount; ++i) {
        listRelease(stacks[i]);
    }
    free(fb.buf);
    free(stacks);
    fclose(fp);
}
