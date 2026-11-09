// threads.md §5 — shared counter: wrong total without a mutex ("nolock"), right with it
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define THREADS 4
#define LOOPS 1000000

long counter = 0;
int use_lock = 1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *inc(void *arg)
{
    (void)arg;
    for (int j = 0; j < LOOPS; j++) {
        if (use_lock)
            pthread_mutex_lock(&lock);
        counter++;                  // read, add, write back: three steps, not one
        if (use_lock)
            pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    use_lock = !(argc > 1 && strcmp(argv[1], "nolock") == 0);
    pthread_t tid[THREADS];
    for (int i = 0; i < THREADS; i++)
        pthread_create(&tid[i], NULL, inc, NULL);
    for (int i = 0; i < THREADS; i++)
        pthread_join(tid[i], NULL);
    printf("%s: counter = %ld (expected %d)\n", use_lock ? "lock  " : "nolock", counter, THREADS * LOOPS);
    return 0;
}
