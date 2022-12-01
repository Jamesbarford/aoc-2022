#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

#define toint(p) ((p)-48)
#define ARRAY_SIZE (3)
static unsigned int top_elves[ARRAY_SIZE] = {0};

int idxOfMin(void) {
    unsigned int min = UINT_MAX;
    int idx = -1;
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        if (top_elves[i] < min) {
            min = top_elves[i];
            idx = i;
        }
    }

    return idx;
}

void addElvesStash(unsigned int calories) {
    int idx = idxOfMin();
    if (idx != -1 && top_elves[idx] < calories) {
        top_elves[idx] = calories;
    }
}

unsigned int sumArray(void) {
    unsigned int acc = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        acc += top_elves[i];
    }
    return acc;
}

unsigned int solve(rFile *rf) {
    char *ptr = rf->buf;
    unsigned int cur = 0u;
    unsigned int acc = 0u;

    while (*ptr) {
        if (*ptr == '\n' && (*(ptr + 1) == '\n' || *(ptr+1) == '\0')) {
            acc += cur;
            addElvesStash(acc);
            cur ^= cur;
            acc ^= acc;
        } else if (*ptr == '\n') {
            acc += cur;
            cur ^= cur;
        } else {
            cur = cur * 10 + toint(*ptr);
        }
        ++ptr;
    }

    return sumArray();
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    printf("%u\n", solve(rf));
    rFileRelease(rf);
}
