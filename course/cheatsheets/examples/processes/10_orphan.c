// processes.md §4 — orphan: the parent exits first, init (or a subreaper) adopts the child
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        printf("child : my parent is %d\n", getppid());
        fflush(stdout);
        sleep(1);                   // meanwhile the parent exits
        printf("child : my parent is now %d (not the original one)\n", getppid());
        return 0;
    }
    printf("parent: my pid is %d, exiting without wait()\n", getpid());
    return 0;
}
