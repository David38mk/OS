// threads.md §7 — one thread per child process waits for it and measures how long it took
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define N 3
pid_t kid[N];
double secs[N];
int code[N];

static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void *waiter(void *arg)
{
    int i = *(int *)arg;
    double t0 = now_s();
    int st;
    waitpid(kid[i], &st, 0);        // blocks until THIS child ends, no polling, no sleep
    secs[i] = now_s() - t0;
    code[i] = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
    return NULL;
}

int main(void)
{
    const char *dur[N] = {"1", "2", "0"};
    for (int i = 0; i < N; i++) {   // 1. fork ALL children first
        kid[i] = fork();
        if (kid[i] < 0) { perror("fork"); return 1; }
        if (kid[i] == 0) {
            execlp("sleep", "sleep", dur[i], (char *)NULL);
            _exit(127);
        }
    }
    pthread_t tid[N];               // 2. only then create the threads
    int idx[N];
    for (int i = 0; i < N; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, waiter, &idx[i]);
    }
    for (int i = 0; i < N; i++)
        pthread_join(tid[i], NULL);
    for (int i = 0; i < N; i++)
        printf("child %d (pid %d): exit code %d, waited %.1f s\n", i, kid[i], code[i], secs[i]);
    return 0;
}
