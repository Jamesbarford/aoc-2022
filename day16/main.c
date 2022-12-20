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

typedef struct dist {
    int name;
    int distance;
} dist;

dist *distNew(int name, int distance) {
    dist *d = malloc(sizeof(dist));
    d->name = name;
    d->distance = distance;
    return d;
}

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

void printPathMapValue(void *ptr) {
    long value = (long)ptr;
    long distance = value >> 32l & 0x7FFFFFFF;
    long flow = value & 0x7FFFFFFF;
    printf(" => distance: %2ld flow: %2ld\n", distance, flow);
}

void printname(int name) {
    printf("%c%c", name >> 8 & 0xFF, name & 0xFF);
}

void printnameKey(int name) {
    printf("key: %c%c => ", name >> 8 & 0xFF, name & 0xFF);
}

mapType path_map = {
    .freeValue = NULL,
    .printValue = printPathMapValue,
    .printKey = printname,
};

mapType cost_to_flow = {
    .freeValue = (MapFree *)mapRelease,
    .printValue = (MapPrintValue *)mapPrint,
    .printKey = printnameKey,
};

mapType sub_cost_to_flow = {
    .freeValue = NULL,
    .printValue = (MapPrintValue *)mapPrint,
    .printKey = printnameKey,
};

mapType int_map = {
        .freeValue = NULL,
        .printValue = NULL,
        .printKey = printint,
};

mapType map_list = {
    .freeValue = (MapFree *)listRelease,
    .printValue = (MapPrintValue *)listPrint,
    .printKey = printint,
};

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
    int cap = BUFSIZ;
    int size = 0;
    char *key = malloc(sizeof(char) * BUFSIZ);

    for (lNode *ln = l->root; ln != NULL; ln = ln->next) {
        if (size + 2 >= cap) {
            cap *= cap;
            key = realloc(key, sizeof(char) * cap);
        }
        size += snprintf(key + size, 4, "%c%c", (char)ln->val >> 8 & 0xFF,
                (char)ln->val & 0xFF);
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

    valve *v = mapGet(nodes, current);
    if (!listHas(visited, current) && v->rate > 0) {
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
    printf("part1: %d\n", answer);
    return answer;
}

map *bfs(map *valves, int start) {
    list *queue = listNew();
    listAppend(queue, (long)start << 32l | 0);
    lNode *ln;
    map *path_costs = mapNew(&path_map);

    while ((ln = listDequeue(queue))) {
        int name = ln->val >> 32L & 0x7FFFFFFF;
        int cost = ln->val & 0x7FFFFFFF;

        valve *v = mapGet(valves, name);
        /*
        printname(name);
        printf(" => %d\n", cost);
        valvePrint(v);
        printf("========\n");
        */

        /* store cost of getting to node along with the flow of that node */
        long cost_flow_rate = (long)cost << 32l | v->rate;

        mapAdd(path_costs,name,(void *)(long)cost_flow_rate);
        for (int i = 0; i < v->count; ++i) {
            if (!mapHas(path_costs,v->connections[i])) {
                listAppend(queue, (long)v->connections[i] << 32l | cost + 1);
            }
        }
        free(ln);
    }
    
    /* Remove flows of 0 */
    for (int i = 0; i < SLAB_SIZE; ++i) {
        mNode *mn = path_costs->slab[i];
        mNode *prev = NULL;

        while (mn) {
            if (((long)mn->value & 0x7FFFFFFF) == 0 && mn->key != start) {
                if (prev) {
                    prev->next = mn->next;
                } else {
                    path_costs->slab[i] = mn->next;
                }
                free(mn);
                path_costs->len--;
            }
            mn=mn->next;
        }
    }
    
    listRelease(queue);
    return path_costs;
}

/**
 * 'CC' => {
 *    'DD' => (long)<int>|<int>
 * }
 * distance => flow
 */
map *createGraph(map *valves) {
    int len;
    int *keys = mapKeys(valves, &len);
    map *path_costs = mapNew(&cost_to_flow);

    for (int i = 0; i < len; ++i) {
        mapAdd(path_costs,keys[i],bfs(valves,keys[i]));
        valve *v = mapGet(valves,keys[i]);

        /* Mark for delete */
        if (v->rate == 0 && v->name != ('A' << 8 | 'A')) {
            keys[i] |= 10 << 16;
        }
    }

    /* clean up */
    for (int i = 0; i < len; ++i) {
        if ((keys[i] >> 16 & 0xFF) == 10) {
            mapDelete(path_costs, keys[i] & 0xFFFF);
        }
    }

    free(keys);
    return path_costs;
}

map *createFlows(map *m) {
    map *flows = mapNew(&int_map);
    for (int i = 0; i < SLAB_SIZE; ++i) {
        mNode *mn = m->slab[i];
        while (mn) {
            valve *v = mn->value;
            if (v && (v->rate > 0 || v->name == 0x4141)) {
                mapAdd(flows,v->name,v);
            }
            mn = mn->next;
        }
    }
    return flows;
}

typedef struct run {
    long flow;
    long minute;
    long name;
    list *visited;
} run; 

run *runNew(long flow, long minute, long name, list *visited) {
    run *r = malloc(sizeof(run));
    r->name = name;
    r->flow = flow;
    r->minute = minute;
    r->visited = visited;
    return r;
}

int runHas(run *r, long needle) {
    return listHas(r->visited,needle);
}

void runRelease(run *r) {
    if (r) {
        listRelease(r->visited);
        free(r);
    }
}

int findMaxForGraph(map *path_costs, int minutes) {
    int max_pressure = 0;
    int start_key = ('A' << 8 | 'A');
    int len;
    list *queue = listNew();
    run *r = runNew(0,0,start_key,listNew());

    for (int i = 0; i < SLAB_SIZE; ++i) {
        mNode *mn = path_costs->slab[i];
        while(mn) {
            if (mn->key != start_key) {
                listAppend(r->visited,mn->key);
            }
            mn=mn->next;
        }
    }

    listAppend(queue,(long)r);

    lNode *cur = NULL;
    while ((cur = listDequeue(queue)) != NULL) {
        r = (run *)cur->val;
        map *links = mapGet(path_costs,r->name);

        for (lNode *ln = r->visited->root; ln != NULL; ln = ln->next) {
            long cost_and_flow = (long)mapGet(links,ln->val);
            long cost = cost_and_flow >> 32 & 0x7FFFFFFF;
            long flow = cost_and_flow & 0x7FFFFFFF;
            long new_flow = r->flow + flow * (minutes - r->minute - cost - 1);

            if (cost >= minutes - r->minute) {
                continue;
            } else {
                list *next_visited = listNew();

                for (lNode *v = r->visited->root; v != NULL; v = v->next) {
                    if (v->val != ln->val) {
                        listAppend(next_visited,v->val);
                    }
                }
                run *new_r = runNew(new_flow,r->minute+cost+1,ln->val,next_visited);
                listAppend(queue,(long)new_r);

                if (new_flow > max_pressure) {
                    max_pressure = new_flow;
                }
            }

        }
        runRelease(r);
    }
    listRelease(queue);

    return max_pressure;
}

int solvePartOne(map *path_costs) {
    return findMaxForGraph(path_costs, 30);
}

int solvePart2FairlyQuickly(map *path_costs) {
    int len;
    int *keys = mapKeys(path_costs,&len);
    list *keyslist = listNew();
    printf("keys: ");
    for (int i = 0; i < len; ++i) {
        if (keys[i] != 0x4141) {
            printf("%c%c, ",keys[i]>>8&0xFF, keys[i]&0xFF);
            listAppend(keyslist,keys[i]);
        }
    }
    printf("\n");


    int actual = 0;
    /* A silly amount of rounds */
    listQsort(keyslist,__lnode_cmp_GTE);
    list **my_goes = listGetAllCombinations(keyslist,len/2,30000,&actual);
    list **elephant_goes = malloc(sizeof(list *) * actual);

    for (int i = 0; i < actual; ++i) {
        elephant_goes[i] = listCopy(keyslist);
    }

    printf("actual combos: %d\n", actual);

    int pressure_max = 0;

    /* Threading would halve the time this takes to run */
    for (int i = 0; i < actual; ++i) {
        list *turn = my_goes[i];
        list *ele_turn = elephant_goes[i];

        /* These map will not free, the child nodes */
        map *mySubGraph = mapNew(&sub_cost_to_flow);
        map *eleSubGraph = mapNew(&sub_cost_to_flow);

        mapAdd(mySubGraph,0x4141,mapGet(path_costs,0x4141));
        mapAdd(eleSubGraph,0x4141,mapGet(path_costs,0x4141));

        for (lNode *ln = turn->root; ln != NULL; ln = ln->next) { 
            listRemove(ele_turn,ln->val);
            mapAdd(mySubGraph,ln->val,mapGet(path_costs,ln->val));
        }

        for (lNode *ln = ele_turn->root; ln != NULL; ln = ln->next) { 
            mapAdd(eleSubGraph,ln->val,mapGet(path_costs,ln->val));
        }

        pressure_max = MAX(findMaxForGraph(mySubGraph,26) + findMaxForGraph(eleSubGraph,26),
                    pressure_max);

        mapRelease(mySubGraph);
        mapRelease(eleSubGraph);
    }

    for (int i = 0; i < actual; ++i) {
        listRelease(my_goes[i]);
        listRelease(elephant_goes[i]);
    }

    listRelease(keyslist);
    free(keys);
    return pressure_max;
}

/* This is a maximum flow problem */
int main(int argc, char **argv) {
    char *file_name = argc == 2 ? argv[1] : "./input.txt";
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

    fclose(fp);
    map *path_costs = createGraph(vmap);
    int part1 = findMaxForGraph(path_costs,30);
    int part2 = solvePart2FairlyQuickly(path_costs);
    printf("part1: %d\n", part1);
    printf("part2: %d\n", part2);

    mapRelease(path_costs);
    mapRelease(vmap);
    

    return 0;
}
