#include <sys/stat.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HASHTAB_SIZ (12)
#define hash(x) ((x) % HASHTAB_SIZ)

/**
 * PART 1 hashtable
 */
static const int hashtab_part1[HASHTAB_SIZ] = {
    [0] = 0,
    [1] = 3, /* 0x5a2041 A Z rock scissors L */
    [2] = 9, /* 0x5a2042 B Z paper scissors W */
    [3] = 6, /* 0x5a2043 C Z scissors scissors D */
    [4] = 0,
    [5] = 4, /* 0x582041 A X rock rock D */
    [6] = 1, /* 0x582042 B X paper rock L */
    [7] = 7, /* 0x582043 C X scissors rock W */
    [8] = 0,
    [9] = 8,  /* 0x592041 A Y rock paper W */
    [10] = 5, /* 0x592042 B Y paper paper D */
    [11] = 2, /* 0x592043 C Y scissors papaer L */
};

/*
static const int hashtab_part2[HASHTAB_SIZ] = {
    [0] = 0,
    [1] = 8, // 0x5a2041 A Z rock paper W
    [2] = 9, // 0x5a2042 B Z paper scissors W
    [3] = 7, // 0x5a2043 C Z scissors rock W
    [4] = 0,
    [5] = 3, // 0x582041 A X rock  scissors L
    [6] = 1, // 0x582042 B X paper rock L
    [7] = 2, // 0x582043 C X scissors paper L
    [8] = 0,
    [9] = 4,  // 0x592041 A Y rock rock D
    [10] = 5, // 0x592042 B Y paper paper D
    [11] = 6, // 0x592043 C Y scissors scissors D
};
*/

static const uint64_t u64tab =
        0b011001010100000000100001001100000111100110000000;

int
main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "r");
    struct stat st;
    char *buffer = NULL, *ptr = NULL;
    unsigned int part1 = 0, part2 = 0, idx = 0;

    assert(fp != NULL);
    assert(stat(argv[1], &st) != -1);
    assert((buffer = malloc(sizeof(char) * st.st_size)) != NULL);
    assert(fread(buffer, sizeof(char), st.st_size, fp) == st.st_size);

    for (ptr = buffer; *ptr; ptr += 4) {
        idx = hash(ptr[0] | (ptr[1] << 8) | (ptr[2] << 16));
        part1 += hashtab_part1[idx];
        part2 += (u64tab >> (idx * 4)) & 0xF;
    }

    printf("part1: %u\n", part1);
    printf("part2: %u\n", part2);
    free(buffer);
    fclose(fp);
}
