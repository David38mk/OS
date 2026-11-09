// threads.md §5 — mutex inside a struct: pthread_mutex_init / destroy, short critical section
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct Shared {
    pthread_mutex_t m;              // a mutex that is not a global needs pthread_mutex_init
    int total;
};

void *add(void *arg)
{
    struct Shared *s = arg;
    for (int i = 0; i < 1000; i++) {
        int mine = i % 7;           // private work stays OUTSIDE the lock
        pthread_mutex_lock(&s->m);
        s->total += mine;           // only the shared update is inside
        pthread_mutex_unlock(&s->m);
    }
    return NULL;
}

int main(void)
{
    struct Shared s = {.total = 0};
    if (pthread_mutex_init(&s.m, NULL) != 0) {
        fprintf(stderr, "mutex init failed\n");
        return 1;
    }
    pthread_t tid[3];
    for (int i = 0; i < 3; i++)
        pthread_create(&tid[i], NULL, add, &s);
    for (int i = 0; i < 3; i++)
        pthread_join(tid[i], NULL);
    pthread_mutex_destroy(&s.m);    // only when no thread can use it any more
    printf("total = %d (expected 8991)\n", s.total);   // 3 threads x 2997
    return 0;
}
