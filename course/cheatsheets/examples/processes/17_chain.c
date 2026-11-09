// processes.md §8 — chain: each child forks the next one (depth 0 = the original process)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define DEPTH 3

int main(void)
{
    int depth = 0;
    printf("depth %d: pid=%d ppid=%d\n", depth, getpid(), getppid());
    fflush(stdout);
    while (depth < DEPTH) {
        pid_t p = fork();
        if (p < 0) {
            perror("fork");
            return 1;
        }
        if (p > 0) {                // parent of this link: wait for the rest of the chain and stop
            waitpid(p, NULL, 0);
            return 0;
        }
        depth++;                    // child: becomes the next link and loops again
        printf("depth %d: pid=%d ppid=%d\n", depth, getpid(), getppid());
        fflush(stdout);
    }
    return 0;
}
