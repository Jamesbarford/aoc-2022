#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define getBit(ch) ((ch) >= 97 && (ch) <= 122 ? (ch)-97 : (ch)-39)

static uint64_t bitvecs[3] = { 0ul, 0ul, 0ul };

int
main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    char buffer[BUFSIZ];
    int part1 = 0, part2 = 0;
    int iter = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        unsigned int len = strlen(buffer);
        uint64_t bitvec = 0;

        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        for (int i = 0; i < len; ++i) {
            bitvecs[iter] |= (uint64_t)(1ul << getBit(buffer[i]));
        }
        iter++;

        if (iter == 3) {
            int setbit = 0;
            uint64_t all = ((uint64_t)bitvecs[0] & (uint64_t)bitvecs[1] &
                    (uint64_t)bitvecs[2]);

            /* Find the set bit in bitmap */
            for (int i = 64; i >= 0; --i) {
                if ((all >> i) & 1ul) {
                    setbit = i + 1;
                    break;
                }
            }

            part2 += setbit;
            iter = 0;
            bitvecs[0] = bitvecs[1] = bitvecs[2] = 0;
        }

        for (int i = 0; i < (len / 2); ++i) {
            bitvec |= (uint64_t)(1ul << getBit(buffer[i]));
        }

        for (int i = len / 2; i < len; ++i) {
            uint64_t bit = getBit(buffer[i]);
            if (bitvec & (uint64_t)(1ul << bit)) {
                part1 += bit + 1;
                break;
            }
        }
    }

    printf("part1: %d\n", part1);
    printf("part2: %d\n", part2);

    fclose(fp);
}
