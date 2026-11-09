// threads.md §3 — two ways to get a result back: malloc'd return value, or a slot in an array
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int slot[3];                        // way 2: global array, one cell per thread

void *square(void *arg)
{
    int i = *(int *)arg;
    int *res = malloc(sizeof *res); // way 1: never return the address of a local variable
    if (res == NULL)
        return NULL;
    *res = i * i;
    slot[i] = i + 100;
    return res;                     // same as pthread_exit(res)
}

int main(void)
{
    pthread_t tid[3];
    int idx[3];
    for (int i = 0; i < 3; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, square, &idx[i]);
    }
    for (int i = 0; i < 3; i++) {
        void *ret;
        pthread_join(tid[i], &ret); // join stores the thread's return value in ret
        if (ret != NULL) {
            printf("thread %d returned %d, slot[%d]=%d\n", i, *(int *)ret, i, slot[i]);
            free(ret);              // the joiner owns the memory now
        }
    }
    return 0;
}
