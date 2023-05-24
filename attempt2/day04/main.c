#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
panic(char *fmt, ...)
{
    va_list ap;
    char buf[BUFSIZ];
    unsigned int len;

    va_start(ap, fmt);

    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    buf[len] = '\0';
    fprintf(stderr, "%s", buf);
    va_end(ap);
    exit(EXIT_FAILURE);
}

int
inrange(int x1, int y1, int x2, int y2)
{
    return x1 >= x2 && y1 <= y2;
}

int
hasoverlap(int x1, int y1, int x2, int y2)
{
    return inrange(x1, y1, x2, y2) || x1 >= x2 && x1 <= y2;
}

int
main(int argc, char **argv)
{
    if (argc != 2) {
        panic("Usage: %s <file>\n", argv[0]);
    }
    FILE *fp = fopen(argv[1], "r");
    char buffer[BUFSIZ];
    int x1, y1, x2, y2;
    int part1 = 0;
    int part2 = 0;

    while (fgets(buffer, sizeof(buffer), fp)) {
        unsigned int len = strlen(buffer);

        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        if (sscanf(buffer, "%d-%d,%d-%d", &x1, &y1, &x2, &y2) == EOF) {
            panic("REACHEd EOF\n");
        }

        if (hasoverlap(x1, y1, x2, y2) || hasoverlap(x2, y2, x1, y1)) {
            part2++;
        }

        if (inrange(x1, y1, x2, y2) || inrange(x2, y2, x1, y1)) {
            part1++;
        }
    }

    printf("part1: %d\n", part1);
    printf("part2: %d\n", part2);
}
