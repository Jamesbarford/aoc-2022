#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
swap(int *x, int *y)
{
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

void
heapifyUp(int *heap, int idx)
{
    while (idx >= 0 && heap[idx / 2] < heap[idx]) {
        swap(&heap[idx / 2], &heap[idx]);
        idx /= 2;
    }
}

void
heapifyDown(int *heap, int idx, int len)
{
    while (idx <= len / 2) {
        int child = idx * 2;

        if (child < len && heap[child + 1] > heap[child ]) {
            child++;
        }

        if (heap[child] <= heap[idx]) {
            break;
        }

        swap(&heap[child], &heap[idx]);
        idx = child;
    }
}

int
heapDelete(int *heap, int *heaplen)
{
    if (*heaplen == 0) {
        return INT_MIN;
    }
    int val = heap[0];
    swap(&heap[*heaplen], &heap[0]);
    *heaplen -= 1;
    heapifyDown(heap, 0, *heaplen);

    return val;
}

void
heapInsert(int *heap, int *heaplen, int value)
{
    heap[*heaplen] = value;
    heapifyUp(heap, *heaplen);
    *heaplen += 1;
}

int
main(int argc, char **argv)
{
    if (argc != 2) {
        return 0;
    }
    struct stat st;
    int acc = 0, num = 0, max = INT_MIN, fd = -1, heaplen = 0;
    char *buffer = NULL, *ptr = NULL;
    int heap[BUFSIZ] = { -1 };

    assert((fd = open(argv[1], O_RDONLY, 0666)) != -1);
    assert(fstat(fd, &st) != -1);
    assert((buffer = malloc(sizeof(char) * st.st_size)) != NULL);
    assert(read(fd, buffer, st.st_size) == st.st_size);

    ptr = buffer;

    while (*ptr) {
        num = 0;
        while (*ptr != '\n') {
            num = num * 10 + (*ptr - 48);
            ptr++;
        }
        acc += num;

        if (*(ptr + 1) == '\n' || *(ptr + 1) == '\0') {
            heapInsert(heap, &heaplen, acc);
            acc = 0;
        }
        ptr++;
    }

    acc = 0;
    max = heapDelete(heap, &heaplen);
    acc += max;
    for (int i = 0; i < 2; ++i) {
        acc += heapDelete(heap, &heaplen);
    }

    printf("day1: %d\n", max);
    printf("day2: %d\n", acc);
    free(buffer);
    close(fd);
}
