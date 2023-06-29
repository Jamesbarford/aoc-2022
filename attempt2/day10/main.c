#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getCycleStrength(int cycle, int reg) {
    switch (cycle) {
    case 20:
    case 60:
    case 100:
    case 140:
    case 180:
    case 220:
        return cycle * reg;
    default:
        return 0;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    char buffer[BUFSIZ];
    int RAX = 1;
    int cycles = 1;
    int strength = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        char *ptr = buffer;

        cycles++;
        strength += getCycleStrength(cycles, RAX);
        if (!strncmp("addx", ptr, 4)) {
            cycles++;
            RAX += atoi(ptr + 5);
        }
        strength += getCycleStrength(cycles, RAX);
    }

    printf("cycles: %d, RAX: %d strength: %d\n", cycles,RAX,strength);
    fclose(fp);
}
