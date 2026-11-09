// processes.md §8 — parent timeout: poll with WNOHANG, kill the child when time is up
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    const int T = 2;                // seconds the child may run
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        execlp("sleep", "sleep", "10", (char *)NULL);
        _exit(127);
    }
    struct timespec t0, t;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (;;) {
        int st;
        pid_t r = waitpid(p, &st, WNOHANG);     // 0 = child still running, p = it ended
        if (r == p) {
            printf("child finished by itself\n");
            break;
        }
        if (r == -1) {
            perror("waitpid");
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &t);
        if (t.tv_sec - t0.tv_sec >= T) {
            kill(p, SIGKILL);
            waitpid(p, &st, 0);                 // reap it, otherwise it stays a zombie
            printf("TIMEOUT after %d s, child killed (signal %d)\n", T, WTERMSIG(st));
            break;
        }
        usleep(100000);                         // poll every 100 ms, not a busy loop
    }
    return 0;
}
