// processes.md §6 — time() has 1 s granularity, clock_gettime(CLOCK_MONOTONIC) does not
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void)
{
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        usleep(300000);             // the child works for 0.3 s
        _exit(0);
    }
    time_t t0 = time(NULL);
    double m0 = now_s();
    waitpid(p, NULL, 0);
    printf("time():      %ld s\n", (long)(time(NULL) - t0));   // 0 or 1, depends on the clock tick
    printf("monotonic:   %.2f s\n", now_s() - m0);
    return 0;
}
