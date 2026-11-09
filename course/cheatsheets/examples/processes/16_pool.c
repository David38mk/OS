// processes.md §8 — pool: N jobs, but at most K children running at the same time
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 6
#define K 2

int main(void)
{
    int running = 0;
    for (int i = 0; i < N; i++) {
        if (running == K) {         // pool is full: wait for any one child to finish
            wait(NULL);
            running--;
        }
        pid_t p = fork();
        if (p < 0) {
            perror("fork");
            break;
        }
        if (p == 0) {
            printf("job %d start\n", i);
            fflush(stdout);
            sleep(1);
            printf("job %d end\n", i);
            fflush(stdout);                 // _exit does NOT flush stdio: do it yourself
            _exit(0);
        }
        running++;
    }
    while (wait(NULL) > 0)          // collect the rest
        ;
    return 0;
}
