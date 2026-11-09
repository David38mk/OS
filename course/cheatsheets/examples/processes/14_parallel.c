// processes.md §8 — N children run in parallel, parent starts all first and then waits for all
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 4

int main(void)
{
    for (int i = 0; i < N; i++) {
        pid_t p = fork();
        if (p < 0) {
            perror("fork");
            break;
        }
        if (p == 0) {
            sleep(N - i);           // child 0 sleeps longest: they finish in reverse order
            printf("child %d done\n", i);
            fflush(stdout);                 // _exit does NOT flush stdio: do it yourself
            _exit(0);
        }
    }
    int reaped = 0;
    while (wait(NULL) > 0)          // wait() returns -1 when there are no children left
        reaped++;
    printf("parent: reaped %d children\n", reaped);
    return 0;
}
