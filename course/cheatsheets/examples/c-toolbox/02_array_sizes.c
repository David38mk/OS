// c-toolbox.md §1 — sizing an array by argc: fixed 100 (exam "max 100") vs VLA vs malloc
#include <stdio.h>
#include <stdlib.h>

#define MAX 100

int main(int argc, char *argv[]) {
    int n = argc - 1;                        // number of file names / values given
    if (n < 1) {
        fprintf(stderr, "Upotreba: %s a b c ...\n", argv[0]);
        return 1;
    }
    if (n > MAX) {                           // the task promises at most 100: still guard it
        fprintf(stderr, "Premnogu argumenti (max %d)\n", MAX);
        return 1;
    }
    int fixed[MAX];                          // always 400 bytes, simple, safe for "max 100"
    int vla[n];                              // size known only at run time; n must be > 0
    int *heap = malloc(n * sizeof(int));     // same idea, lives until free(); check for NULL
    if (heap == NULL) {
        perror("malloc");
        return 1;
    }
    for (int i = 0; i < n; i++)
        fixed[i] = vla[i] = heap[i] = i * i;
    printf("n=%d sizeof(fixed)=%zu sizeof(vla)=%zu last=%d %d %d\n", n, sizeof(fixed),
           sizeof(vla), fixed[n - 1], vla[n - 1], heap[n - 1]);
    free(heap);
    return 0;
}
