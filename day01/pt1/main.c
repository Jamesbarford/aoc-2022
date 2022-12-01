#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../includes/readfile.h"

#define toint(p) ((p)-48)

unsigned int solve(rFile *rf) {
    char *ptr = rf->buf;
    unsigned int elf_with_most = 0u;
    unsigned int cur = 0u;
    unsigned int acc = 0u;

    while (*ptr) {
        if (*ptr == '\n' && *(ptr+1) == '\n') {
            acc += cur;
            cur ^= cur;
            if (acc > elf_with_most) {
                elf_with_most = acc;
            }
            acc ^= acc;
        } else if (*ptr == '\n') {
            acc += cur;
            cur ^= cur;
        } else {
            cur = cur * 10 + toint(*ptr);
        }
        ++ptr;
    }

    return elf_with_most;
}

int main(void) {
    rFile *rf = rFileRead("./input.txt");
    solve(rf);
    printf("%u\n", solve(rf));
    rFileRelease(rf);
}
