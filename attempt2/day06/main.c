#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int solve(char *buffer, int len, int som_count) {
    int p1 = 0, c;
    
    while (1) {
        unsigned int bitvec = 0;
        int offset = p1 + som_count;
        int bitcount = 0;

        for (int i = p1; i < offset; ++i) {
            c = buffer[i] - 'a';
            if ((bitvec >> c) & 1) {
                break;
            } else {
                bitvec |= 1 << c;
                bitcount++;
            }
        }
        if (bitcount == som_count) {
            break;
        }
        p1++;
    }

    return p1+som_count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    char buffer[10000];
    int idx;

    while (fgets(buffer, sizeof(buffer), fp)) {
        int len = strlen(buffer);

        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        idx = solve(buffer, len, 4);
        printf("part1: [%d] %c\n",idx, buffer[idx]);
        idx = solve(buffer, len, 14);
        printf("part2: [%d] %c\n",idx, buffer[idx]);
    }
    fclose(fp);
}
