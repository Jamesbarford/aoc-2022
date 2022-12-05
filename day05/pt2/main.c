#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

#define toint(p) ((p)-48)
#define null_val ('\0')

typedef struct lNode {
    unsigned long value;
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

void listAppend(list *l, unsigned long value) {
    lNode *ln = malloc(sizeof(lNode));
    ln->value = value;

    if (l->root == NULL) {
        l->root = l->tail = ln;
    } else {
        l->tail->next = ln;
        l->tail = ln;
    }

    l->size++;
}

void listRelease(list *l) {
    if (l) {
        lNode *ln = l->root;
        lNode *cur = NULL;
        while (ln) {
            cur = ln;
            ln = ln->next;
            free(cur);
        }
        free(l);
    }
}

void listPrint(list *l) {
    lNode *ln = l->root;
    printf("[");
    while (ln) {
        printf("(%lu %lu %lu)", ln->value >> 16 & 0xFF, ln->value >> 8 & 0xFF,
                ln->value & 0xFF);
        if (ln->next != NULL) {
            printf(", ");
        }
        ln = ln->next;
    }
    printf("]\n");
}

typedef struct sNode {
    unsigned char value;
    struct sNode *next;
} sNode;

typedef struct stack {
    int size;
    sNode *root;
} stack;

stack *stackNew(void) {
    stack *s = malloc(sizeof(stack));
    s->root = NULL;
    s->size = 0;
    return s;
}

void stackPush(stack *s, int value) {
    sNode *sn = malloc(sizeof(sNode));
    sn->value = value;
    sn->next = s->root;
    s->root = sn;
    s->size++;
}

unsigned char stackPop(stack *s) {
    if (s->size == 0) {
        return null_val;
    }

    sNode *rem = s->root;
    unsigned char value = rem->value;
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

void stackPrint(stack *s) {
    sNode *sn = s->root;
    printf("[");
    while (sn) {
        printf("%c", sn->value);
        if (sn->next != NULL) {
            printf(", ");
        }
        sn = sn->next;
    }
    printf("]\n");
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

int advanceToDigit(char *ptr) {
    int offset = 0;

    while (!isdigit(*ptr)) {
        ++ptr;
        ++offset;
    }

    return offset;
}

/**
 * store count from & 2 as a bitmask
 */
list *initList(rFile *rf) {
    list *l = listNew();
    char *ptr = rf->buf;
    int offset = advanceTil(ptr, 'm');
    ptr += offset;
    unsigned long instruction = 0UL;
    unsigned long tmp = 0UL;

    while (*ptr != '\0') {
        ptr += advanceToDigit(ptr);
        while (isdigit(*ptr)) {
            tmp = tmp * 10 + toint(*ptr);
            ++ptr;
        }

        instruction |= tmp << 16;
        tmp = 0;

        ptr += advanceToDigit(ptr);
        while (isdigit(*ptr)) {
            tmp = tmp * 10 + toint(*ptr);
            ++ptr;
        }

        instruction |= tmp << 8;
        tmp = 0;

        ptr += advanceToDigit(ptr);
        while (isdigit(*ptr)) {
            tmp = tmp * 10 + toint(*ptr);
            ++ptr;
        }
        instruction |= tmp;
        tmp = 0;

        listAppend(l, instruction);
        instruction = 0;
        ++ptr;
    }

    return l;
}

unsigned long listDequeue(list *l) {
    if (l->size == 0) {
        return 0;
    }

    lNode *rem = l->root;
    unsigned long value = rem->value;
    l->root = rem->next;
    l->size--;
    free(rem);
    return value;
}

stack **initStacks(rFile *rf, int *stack_count) {
    int offset = 0;
    *stack_count = 0;
    stack **arr = NULL;
    char *ptr = rf->buf;

    offset = advanceTil(ptr, '1');
    ptr += offset;

    while (*ptr != '\n') {
        if (isdigit(*ptr)) {
            (*stack_count)++;
        }
        ++ptr;
    }

    arr = malloc(*stack_count * sizeof(stack *));
    for (int i = 0; i < *stack_count; ++i) {
        arr[i] = stackNew();
    }

    return arr;
}

void releaseStackArray(stack **arr, int count) {
    if (count == 0 || arr == NULL) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        free(arr[i]);
    }
    free(arr);
}

char *solve(rFile *rf) {
    int stack_count = 0;
    list *instruction_set = initList(rf);
    stack **arr = initStacks(rf, &stack_count);
    char *outstring = malloc(sizeof(char) * stack_count + 1);

    /*   ' '   ' '
     *[c]' '[c]' '...\n
     * each column is 4 chars wide
     */
    char *ptr = rf->buf;
    int char_pos = 0;
    int stack_idx = 0;
    char c = '\0';
    while (!isdigit(*ptr)) {
        if (*ptr == '\n') {
            char_pos = 0;
            ++ptr;
            continue;
        } else if (*ptr == '[') {
            c = *(ptr + 1);
            stack_idx = (char_pos + 3) / 4;
            stackPush(arr[stack_idx], c);
        }

        ++char_pos;
        ++ptr;
    }

    /* Stacks are the wrong way round */
    for (int i = 0; i < stack_count; ++i) {
        stackReverse(arr[i]);
    }

    for (int i = 0; i < stack_count; ++i) {
        stackPrint(arr[i]);
    }

    unsigned long instruction = 0U;
    while ((instruction = listDequeue(instruction_set)) != 0) {
        unsigned long count = instruction >> 16 & 0XFF;
        unsigned long from = instruction >> 8 & 0XFF;
        unsigned long to = instruction & 0XFF;

        printf("move %lu from %lu to %lu\n", count, from, to);

        stack *src = arr[from - 1];
        stack *dest = arr[to - 1];

        stack *tmp = stackNew();

        while (count--) {
            unsigned char val = stackPop(src);
            stackPush(tmp, val);
        }

        unsigned char val = null_val;
        while ((val = stackPop(tmp)) != null_val) {
            stackPush(dest, val);
        }
        stackRelease(tmp);

        for (int i = 0; i < stack_count; ++i) {
            stackPrint(arr[i]);
        }
        printf("-------\n");
    }
    free(instruction_set);

    for (int i = 0; i < stack_count; ++i) {
        stackPrint(arr[i]);
    }

    for (int i = 0; i < stack_count; ++i) {
        unsigned char val = stackPop(arr[i]);
        outstring[i] = val;
    }

    listRelease(instruction_set);
    releaseStackArray(arr, stack_count);

    outstring[stack_count] = '\0';
    return outstring;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    char *solution = solve(rf);
    printf("%s\n", solution);
    free(solution);
    rFileRelease(rf);
}
