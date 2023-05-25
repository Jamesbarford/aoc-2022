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
main(int argc, char **argv)
{
    if (argc != 2) {
        panic("Usage: %s <file>\n", argv[0]);
    }
    FILE *fp = fopen(argv[1], "r");
    char buffer[BUFSIZ];

    while (fgets(buffer, sizeof(buffer), fp)) {
        unsigned int len = strlen(buffer);

        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
    }
}
