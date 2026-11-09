// threads.md §2 — create 4 threads, give each its own index, join them all
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *work(void *arg)
{
    int i = *(int *)arg;            // read the index BEFORE main can change it
    printf("thread %d running\n", i);
    return NULL;
}

int main(void)
{
    pthread_t tid[4];
    int idx[4];                     // one slot per thread: each thread gets its own address
    for (int i = 0; i < 4; i++) {
        idx[i] = i;
        int rc = pthread_create(&tid[i], NULL, work, &idx[i]);
        if (rc != 0) {              // pthread functions RETURN the error, they do not set errno
            fprintf(stderr, "pthread_create: error %d\n", rc);
            return 1;
        }
    }
    for (int i = 0; i < 4; i++)
        pthread_join(tid[i], NULL);
    printf("all joined\n");
    return 0;
}
