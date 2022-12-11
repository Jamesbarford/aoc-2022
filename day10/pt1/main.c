#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getCycleStrength(int cycle, int Xreg) {
    switch (cycle) {
        case 20:
        case 60:
        case 100:
        case 140:
        case 180:
        case 220:
            return cycle * Xreg;
            break;
    }
    return 0;
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[64] = {0};
    int Xreg = 1;
    int tmp = 0;
    int should_calc = 0;
    int cycle = 1;
    int strength = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        char *ptr = buf;
        int isnoop = strncasecmp(ptr, "noop", 4) == 0;
        int isaddx = strncasecmp(ptr, "addx", 4) == 0;

        if (isaddx) {
            cycle += 1;
            strength += getCycleStrength(cycle, Xreg);
            ptr += 5;
        } else if (isnoop) {
            cycle += 1;
            strength += getCycleStrength(cycle, Xreg);
            continue;
        } else {
            fprintf(stderr, "Invalid command: %s\n", buf);
            exit(1);
        }

        // printf("[%d]->%s", cycle, buf);

        int num = (int)strtoll(ptr, NULL, 10);
        if (tmp > 1) {
            // acc += tmp;
        }

        // printf("[%d] %d + %d\n", cycle, num, Xreg);

        Xreg += num;
        cycle += 1;
            strength += getCycleStrength(cycle, Xreg);

    }
    int s;
    s = getCycleStrength(cycle, Xreg);
    if (s != 0) {
        printf("strength : %d\n", s);
    }
    strength += getCycleStrength(cycle, Xreg);
    fclose(fp);
    printf("[%d]: %d strength: %d\n", cycle, Xreg, strength);
    return 0;
}
