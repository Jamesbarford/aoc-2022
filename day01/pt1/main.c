#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

long long getmax(long long *arr, int len) {
    long long m = LLONG_MIN;
    for (int i = 0; i < len; ++i) {
        if (arr[i] > m) {
            m = arr[i];
        }
    }
    return m;
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[BUFSIZ] = {'\0'};
    long long mem[BUFSIZ] = {0};
    int ptr = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        long long l = strtoll(buf, NULL, 10);
        if (l == 0) {
            ++ptr;
        } else {
            mem[ptr] += l;
        }
    }

    fclose(fp);
    printf("%llu\n", getmax(mem, ptr));
}
