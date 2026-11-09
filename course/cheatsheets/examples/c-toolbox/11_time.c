// c-toolbox.md §6 — time/difftime, clock_gettime(CLOCK_MONOTONIC), nanosleep, usleep, why time() is too coarse
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static double now_s(void) {                         // seconds with fractions, never jumps backwards
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(void) {
    time_t t0 = time(NULL);                         // whole seconds since 1970 (wall clock)
    double m0 = now_s();
    struct timespec pauza = {0, 300000000L};        // 0 s + 300,000,000 ns = 0.3 s
    nanosleep(&pauza, NULL);                        // second argument: remaining time if interrupted
    usleep(200000);                                 // microseconds; old, but short to write
    double m1 = now_s();
    time_t t1 = time(NULL);
    printf("monotonic: %.1f s\n", m1 - m0);         // about 0.5
    printf("time():    %.0f s\n", difftime(t1, t0));// 0 or 1: whole seconds, depends on where you start
    sleep(1);                                       // whole seconds only
    printf("after sleep(1): %.1f s\n", now_s() - m0);

    int T = 2;                                      // timeout in whole seconds, like Lab 2 Z2
    double start = now_s();
    while (now_s() - start < T)                     // fine-grained loop: dies within ~0.1 s of T
        usleep(100000);
    printf("timeout reached after %.1f s\n", now_s() - start);
    return 0;
}
