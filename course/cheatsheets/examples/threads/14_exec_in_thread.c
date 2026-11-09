// threads.md §8 — exec in a thread replaces the WHOLE process: main and the other threads vanish
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *runner(void *arg)
{
    (void)arg;
    sleep(1);
    printf("thread: calling execlp, nothing after it in this process will ever run\n");
    fflush(stdout);
    execlp("echo", "echo", "echo: I am now the whole process", (char *)NULL);
    perror("execlp");
    return NULL;
}

int main(void)
{
    pthread_t t;
    pthread_create(&t, NULL, runner, NULL);
    pthread_join(t, NULL);          // never returns when exec succeeds
    printf("main: this line is never printed\n");
    return 0;
}
