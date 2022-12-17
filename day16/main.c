#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmap.h"
#include "list.h"
#include "map.h"

#define MAX_CONNECTIONS 32
#define MAX_VALVES 60
#define MAX(x, y) ((x) > (y) ? (x) : (y))

typedef struct connections {
    char *key;
    int rate;
} connections;

typedef struct valve {
    /* 4 bits in a char so this is excessive */
    int name;         /* 2 chars OR'd into a long */
    int rate;         /* Flow rate */
    int count;        /* number of connections */
    int *connections; /* connections to valves */
} valve;

void valveInit(valve *v) {
    v->name = 0;
    v->rate = 0;
    v->count = 0;
    v->connections = malloc(sizeof(int) * MAX_CONNECTIONS);
}

valve *valveNew(void) {
    valve *v = malloc(sizeof(valve));
    valveInit(v);
    return v;
}

void valveFree(valve *v) {
    if (v->connections) {
        free(v->connections);
    }
}

valve *valveFind(valve *valves, int count, int key) {
    for (int i = 0; i < count; ++i) {
        if (valves[i].name == key) {
            return &valves[i];
        }
    }
    return NULL;
}

void valvePrint(valve *v) {
    if (!v) {
        return;
    }
    printf("%c%c flow: %2d connections: ", v->name >> 8 & 0xFF,
            (v->name & 0xFF), v->rate);
    for (int i = 0; i < v->count; ++i) {
        int vname = v->connections[i];
        printf("%c%c", vname >> 8 & 0xFF, vname & 0xFF);
        if (i + 1 != v->count) {
            printf(", ");
        }
    }
    printf("\n");
}

mapType valve_map = {
        .freeValue = (MapFree *)valveFree,
        .printValue = (MapPrintValue *)valvePrint,
        .printKey = NULL,
};

void printint(int v) {
    printf("%d", v);
}

mapType int_map = {
        .freeValue = NULL,
        .printValue = NULL,
        .printKey = printint,
};

void printname(int name) {
    printf("%c%c", name >> 8 & 0xFF, name & 0xFF);
}

int munchChar(char *ptr, char untill) {
    int offset = 0;
    while (*ptr != untill) {
        ++offset;
        ++ptr;
    }
    return offset;
}

int extractInteger(char *ptr, int *outnum) {
    int offset = 0;
    *outnum = 0;
    while (isdigit(*ptr)) {
        *outnum = *outnum * 10 + *ptr - 48;
        ++ptr;
    }
    return offset;
}

int __list_sort_firstchar(lNode *n1, lNode *n2) {
    char c1 = n1->val >> 8 & 0xFF;
    char c2 = n2->val >> 8 & 0xFF;
    return c1 < c2;
}

char *listToKey(list *l, int *len) {
    // listQsort(l,__list_sort_firstchar);
    int cap = BUFSIZ;
    int size = 0;
    char *key = malloc(sizeof(char) * BUFSIZ);

    for (lNode *ln = l->root; ln != NULL; ln = ln->next) {
        if (size + 2 >= cap) {
            cap *= cap;
            key = realloc(key, sizeof(char) * cap);
        }
        size += snprintf(key + size, 4, "%c%c", ln->val >> 8 & 0xFF,
                ln->val & 0xFF);
    }
    *len = size;
    key[*len] = '\0';
    return key;
}

int __driver(map *nodes, list *visited, hmap *cache, int run, int current,
        int minutes)
{
    if (minutes == 0) {
        return 0;
    }

    char tmp[BUFSIZ] = {0};
    int len, tmplen;
    char *key = listToKey(visited, &len);

    /* Create key for hashtable */
    tmplen = snprintf(tmp, BUFSIZ, "%c%c%d%s", current >> 8 & 0xFF,
            current & 0xFF, minutes, key);
    tmp[tmplen] = '\0';

    list *prev = hmapGet(cache, tmp);

    free(key);

    if (prev != NULL) {
        return prev->priv;
    }

    int pressure = 0;
    int found = 0;

    valve *v = mapGet(nodes, current);
    if (!listHas(visited, current) && v->rate > 0) {
        found = 1;
        list *next_visited = listNew();
        for (lNode *ln = visited->root; ln != NULL; ln = ln->next) {
            listAppend(next_visited, ln->val);
        }
        listAppend(next_visited, current);

        pressure = MAX(pressure,
                v->rate * (minutes - 1) + __driver(nodes, next_visited, cache,
                                                  pressure, current,
                                                  minutes - 1));
    }
    /* if current run has not visited current key and the flow is > 0
     * Start a new run
     * */

    /* Otherwise for each tunnel find the maximum pressure for each one */
    for (int i = 0; i < v->count; ++i) {
        pressure =
                MAX(pressure, __driver(nodes, visited, cache, pressure,
                                      v->connections[i], minutes - 1));
    }

    visited->priv = pressure;
    hmapAdd(cache, strdup(tmp), visited);
    return pressure;
}

/* Too crap for part 2*/
int part1Solution(map *nodes, int current) {
    list *visited = listNew();
    hmap *cache = hmapNew(NULL);
    int answer = __driver(nodes, visited, cache, 0, current, 30);
    listRelease(visited);
    printf("cache: %d\n", cache->size);
    printf("%d\n", answer);
    return answer;
}

/* This is a maximum flow problem */
int main(int argc, char **argv) {
    char *file_name = argc == 2 ? argv[1] : "./warmup.txt";
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(1);
    }
    char tmp[BUFSIZ];
    int valve_count = 0;
    map *vmap = mapNew(&valve_map);

    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t len = strlen(tmp);
        if (tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
            --len;
        }
        char *ptr = tmp;
        valve *v = valveNew();

        valve_count++;

        /* Get current valve */
        ptr += munchChar(ptr, ' ');
        ptr++;
        v->name |= *(ptr++) << 8;
        v->name |= *(ptr++);

        /* Get current valve flow */
        ptr += munchChar(ptr, '=');
        ptr++;
        ptr += extractInteger(ptr, &v->rate);

        /* Get each tunnel leading to a valve */

        /* tunnel|s| <- s of tunnels */
        ptr += munchChar(ptr, 's');
        ptr++;
        /* valves|s| <- s of valves */
        ptr += munchChar(ptr, 'v');
        ptr++;
        ptr += munchChar(ptr, 'e');
        if (*(ptr + 1) == 's') {
            ptr++;
        }

        /* Jump passed space to first character */
        ptr += 2;
        while (*ptr) {
            switch (*ptr) {
                case ',':
                    v->count++;
                    break;
                default: {
                    int name = 0;
                    if (isalpha(*ptr)) {
                        name |= (int)(*ptr << 8);
                        ptr++;
                        name |= (int)(*ptr);
                    }
                    v->connections[v->count] = name;
                    break;
                }
            }
            ++ptr;
        }
        v->count++;
        mapAdd(vmap, v->name, v);
    }

    int key = 0;
    key |= 'A' << 8;
    key |= 'A';
    part1Solution(vmap, key);

    fclose(fp);

    return 0;
}
