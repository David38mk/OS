// threads.md §2 — the &i trap: all threads share the one variable i (run with "bad")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *work(void *arg)
{
    usleep(1000);                   // by now the loop in main has moved on
    printf("thread got %d\n", *(int *)arg);
    return NULL;
}

int main(int argc, char *argv[])
{
    int bad = (argc > 1 && strcmp(argv[1], "bad") == 0);
    pthread_t tid[4];
    int idx[4];
    int i;
    for (i = 0; i < 4; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, work, bad ? (void *)&i : (void *)&idx[i]);
    }
    for (int j = 0; j < 4; j++)
        pthread_join(tid[j], NULL);
    return 0;
}
