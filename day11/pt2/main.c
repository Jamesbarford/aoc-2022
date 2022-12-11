#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define toint(p) ((p)-48)
#define MONKEY_COUNT (10)
#define ROUNDS (10000)

int __qsidx(long long *arr, int high, int low) {
    long long pivot = arr[high];
    int idx = low - 1;

    for (int i = low; i < high; ++i) {
        if (arr[i] <= pivot) {
            ++idx;
            long long tmp = arr[i];
            arr[i] = arr[idx];
            arr[idx] = tmp;
        }
    }
    ++idx;
    arr[high] = arr[idx];
    arr[idx] = pivot;

    return idx;
}

void __qs(long long *arr, int high, int low) {
    if (low >= high) {
        return;
    }

    int pidx = __qsidx(arr, high, low);
    __qs(arr, pidx - 1, low);
    __qs(arr, high, low + 1);
}

void quicksort(long long *arr, int len) {
    __qs(arr, len - 1, 0);
}

typedef struct lNode {
    long long value;
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

long long listPopHead(list *l) {
    if (l->size == 0) {
        return -1;
    }
    lNode *head = l->head;
    long long rem = head->value;
    l->head = head->next;
    free(head);
    l->size--;
    return rem;
}

void listRemove(list *l, int value) {
    lNode *cur = l->head;
    lNode *prev = NULL;
    lNode *n1 = NULL;

    if (cur->value == value) {
        listPopHead(l);
        return;
    }

    prev = cur;
    cur = cur->next;

    while (cur) {
        if (cur->value == value) {
            n1 = cur;
            prev->next = cur->next;
            free(n1);
            l->size--;
            break;
        }
        cur = cur->next;
    }
}

void listAppend(list *l, long long value) {
    lNode *n = malloc(sizeof(lNode));
    n->value = value;
    n->next = NULL;

    if (l->size == 0) {
        l->tail = l->head = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    l->size++;
}

void listClear(list *l) {
    if (l) {
        lNode *cur = l->head;
        lNode *next = NULL;
        while (cur && l->size) {
            next = cur->next;
            free(cur);
            cur = next;
            l->size--;
        }
        l->head = NULL;
    }
}

void listRelease(list *l) {
    if (l) {
        listClear(l);
        free(l);
    }
}

int advancePastWhitespace(char *ptr) {
    int offset = 0;
    while (isspace(*ptr)) {
        ++ptr;
        ++offset;
    }
    return offset;
}

int advanceTo(char *ptr, char character) {
    int offset = 0;
    while (*ptr != character) {
        ++ptr;
        ++offset;
    }
    return offset;
}

enum OP {
    ADD,
    SUB,
    MUL,
    DIV,
    SQUARE,
};

char opchar[5] = {
        [ADD] = '+',
        [SUB] = '-',
        [MUL] = '*',
        [DIV] = '/',
        [SQUARE] = '[',
};

typedef struct monkey {
    list *items;
    int id;
    enum OP op;
    long long op_scalar;
    long long divisor;
    int true_monkey;
    int false_monkey;
} monkey;

monkey *monkeyNew(int id) {
    monkey *m = malloc(sizeof(monkey));
    m->id = id;
    m->items = listNew();
    m->op = ADD;
    m->op_scalar = 0;
    m->divisor = 0;
    m->true_monkey = 0;
    m->false_monkey = 0;
    return m;
}

void monkeyPrint(monkey *m) {
    printf("Monkey %d:\n", m->id);
    lNode *ln = m->items->head;
    while (ln) {
        printf("%lld ", ln->value);
        ln = ln->next;
    }
    printf("\n");
    printf("Operation: %c ", opchar[m->op]);
    if (m->op_scalar > -1) {
        printf("%lld\n", m->op_scalar);
    } else {
        printf("square\n");
    }

    printf("Test: divisable by: %lld\n", m->divisor);
    printf("If true goto: %d\n", m->true_monkey);
    printf("If false goto: %d\n", m->false_monkey);
}

void parseOperation(char *ptr, monkey *m) {
    ptr += advancePastWhitespace(ptr);
    switch (*ptr) {
        case '*':
            m->op = MUL;
            break;
        case '-':
            m->op = SUB;
            break;
        case '+':
            m->op = ADD;
            break;
        case '/':
            m->op = DIV;
            break;
    }

    ptr++;
    ptr += advancePastWhitespace(ptr);
    m->op_scalar = 0;

    if (isdigit(*ptr)) {
        while (isdigit(*ptr)) {
            m->op_scalar = m->op_scalar * 10 + toint(*ptr);
            ++ptr;
        }
    } else if (*ptr == 'o') {
        m->op = SQUARE;
        m->op_scalar = -1;
    }
}

long long monkeyDoOp(monkey *m, long long value) {
    switch (m->op) {
        case ADD:
            return value + m->op_scalar;
        case SUB:
            return value - m->op_scalar;
        case MUL:
            return value * m->op_scalar;
        case DIV:
            return value / m->op_scalar;
        case SQUARE:
            return value * value;
        default:
            fprintf(stderr, "Invalid op: %d\n", m->op);
            exit(1);
    }
}

long long solve(monkey **monkeys, int count, int rounds) {
    long long touches[MONKEY_COUNT] = {0};

    long long destress_amount = 1;
    for (int i = 0; i < count; ++i) {
        destress_amount *= monkeys[i]->divisor;
    }

    monkey *m = NULL;

    for (int r = 0; r < rounds; ++r) {
        for (int i = 0; i < count; ++i) {
            m = monkeys[i];
            list *l = m->items;

            for (lNode *ln = l->head; ln != NULL && l->size; ln = ln->next) {
                long long newval = monkeyDoOp(m, ln->value);
                newval = newval % destress_amount;

                if (newval % m->divisor == 0) {
                    listAppend(monkeys[m->true_monkey]->items, newval);
                } else {
                    listAppend(monkeys[m->false_monkey]->items, newval);
                }
                touches[i]++;
            }
            listClear(l);
        }
    }

    for (int i = 0; i < count; ++i) {
        printf("[%d]: %lld\n", i, touches[i]);
    }

    quicksort(touches, count);
    return touches[count - 1] * touches[count - 2];
}

/* Parse input then datastructure it */
int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[200] = {0};
    int num_monkies = 0;
    int monkey_idx = 0;
    monkey *monkeys[MONKEY_COUNT];
    monkey *cur_monkey = NULL;

    while (fgets(buf, sizeof(buf), fp)) {
        char *ptr = buf;
        ptr += advancePastWhitespace(ptr);
        if (strncasecmp("monkey", buf, 6) == 0) {
            ptr += advanceTo(ptr, ' ');
            ptr++;
            monkey_idx = toint(*ptr);
            monkeys[monkey_idx] = monkeyNew(monkey_idx);
            cur_monkey = monkeys[monkey_idx];
            num_monkies++;
        } else if (strncasecmp("starting items:", ptr, 15) == 0) {
            ptr += advanceTo(ptr, ':');
            ptr += advanceTo(ptr, ' ');
            int cur_num = 0;
            while (*(ptr++) != '\0') {
                switch (*ptr) {
                    case ',':
                    case '\n':
                        listAppend(cur_monkey->items, cur_num);
                        cur_num = 0;
                        break;
                    default:
                        if (isdigit(*ptr)) {
                            cur_num = cur_num * 10 + toint(*ptr);
                        }
                        break;
                }
            }

        } else if (strncasecmp("operation:", ptr, 10) == 0) {
            ptr += advanceTo(ptr, 'd');
            ptr++;
            parseOperation(ptr, cur_monkey);
        } else if (strncasecmp("test:", ptr, 5) == 0) {
            /* "divisible by"<-*/
            ptr += advanceTo(ptr, 'y');
            ptr++;
            ptr += advancePastWhitespace(ptr);
            while (isdigit(*ptr)) {
                cur_monkey->divisor = cur_monkey->divisor * 10 + toint(*ptr);
                ptr++;
            }
        } else if (strncasecmp("if true:", ptr, 8) == 0) {
            /* "throw to monkey"<-*/
            ptr += advanceTo(ptr, 'y');
            ptr++;
            ptr += advancePastWhitespace(ptr);
            cur_monkey->true_monkey = toint(*ptr);
        } else if (strncasecmp("if false:", ptr, 9) == 0) {
            /* "throw to monkey"<-*/
            ptr += advanceTo(ptr, 'y');
            ptr++;
            ptr += advancePastWhitespace(ptr);
            cur_monkey->false_monkey = toint(*ptr);
        }
    }

    printf("-----------\n");
    for (int i = 0; i < num_monkies; ++i) {
        monkeyPrint(monkeys[i]);
        printf("-----------\n");
    }

    fclose(fp);
    printf("answer: %lld\n", solve(monkeys, num_monkies, ROUNDS));
    for (int i = 0; i < num_monkies; ++i) {
        monkey *m = monkeys[i];
        listRelease(m->items);
        free(m);
    }

    return 0;
}
