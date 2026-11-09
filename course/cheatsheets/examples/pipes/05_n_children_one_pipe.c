// pipes.md §3 — N children write one fixed-size record each into ONE pipe, parent reads N records
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 4
struct result { int child; long sum; };            // 16 bytes, far below PIPE_BUF (4096)

int main(void) {
    int fd[2];
    pipe(fd);

    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            close(fd[0]);                          // child only writes
            struct result r = { i, 0 };
            for (int k = 1; k <= (i + 1) * 100; k++) r.sum += k;   // 1+2+...+100*(i+1)
            write(fd[1], &r, sizeof r);            // one write = one atomic record
            close(fd[1]);
            return 0;
        }
    }
    close(fd[1]);                                  // parent's write end: close BEFORE reading
    struct result r;
    int got = 0;
    while (read(fd[0], &r, sizeof r) == sizeof r) {   // loop ends at EOF (all writers gone)
        printf("parent: child %d sum=%ld\n", r.child, r.sum);
        got++;
    }
    close(fd[0]);
    while (wait(NULL) > 0) {}
    printf("parent: %d results (arrival order is not child order)\n", got);
    return 0;
}
