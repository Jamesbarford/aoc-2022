#include <stdio.h>
#include <stdlib.h>

void swap(long long *x, long long *y) {
    long long tmp = *x;
    *x = *y;
    *y = tmp;
}

int main(void) {
    FILE *fp = fopen("./input.txt", "r");
    char buf[BUFSIZ] = {'\0'};
    long long mem[BUFSIZ] = {0};
    long long heap[BUFSIZ] = {0};
    int heapsize = 0;
    int ptr = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        long long l = strtoll(buf, NULL, 10);
        if (l == 0) {
            heap[++heapsize] = mem[ptr];

            int cur = heapsize;
            while (cur && heap[cur / 2] < heap[cur]) {
                int parent = cur / 2;
                swap(&heap[parent], &heap[cur]);
                cur = parent;
            }
            ++ptr;
        } else {
            mem[ptr] += l;
        }
    }

    /* REMOVE the top three from the heap*/
    long long acc = 0;
    for (int i = 0; i < 3; ++i) {
        long long val = heap[0];
        acc += val;

        swap(&heap[heapsize], &heap[0]);
        --heapsize;
        int cur = 0;
        while (cur * 2 <= heapsize) {
            int child = cur * 2;
            if (cur < heapsize && heap[child + 1] > heap[child]) {
                ++child;
            }

            if (heap[cur] >= heap[child]) {
                break;
            }

            swap(&heap[cur], &heap[child]);
            cur = child;
        }
    }

    fclose(fp);
    printf("heap: %lld\n", acc);
}
