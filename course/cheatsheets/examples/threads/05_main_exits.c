// threads.md §3 — when main returns, the whole process ends and kills the other threads
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *slow(void *arg)
{
    (void)arg;
    sleep(1);
    printf("slow thread finished\n");   // never printed if main does not wait
    return NULL;
}

int main(int argc, char *argv[])
{
    (void)argv;
    pthread_t t;
    pthread_create(&t, NULL, slow, NULL);
    if (argc > 1)
        pthread_join(t, NULL);      // correct: wait for the thread (run with any argument)
    printf("main returns now\n");
    return 0;                       // without the join the process dies here, thread and all
}
