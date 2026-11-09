// threads.md §4 — read-only shared input + one private output slot per thread: no lock needed
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define T 4
#define PER 250

int data[T * PER];                  // shared, only READ by the threads
long partial[T];                    // thread i writes ONLY partial[i]

void *sum_part(void *arg)
{
    int i = *(int *)arg;
    long s = 0;
    for (int j = i * PER; j < (i + 1) * PER; j++)
        s += data[j];
    partial[i] = s;
    return NULL;
}

int main(void)
{
    for (int j = 0; j < T * PER; j++)
        data[j] = j % 10;           // filled by main BEFORE the threads exist
    pthread_t tid[T];
    int idx[T];
    for (int i = 0; i < T; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, sum_part, &idx[i]);
    }
    long total = 0;
    for (int i = 0; i < T; i++) {
        pthread_join(tid[i], NULL);
        total += partial[i];        // read partials only after join: they are final
    }
    printf("total = %ld (expected %d)\n", total, T * PER / 10 * 45);
    return 0;
}
