// processes.md §1 — fork copies memory: the child changes its copy, the parent never sees it
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int counter = 10;

int main(void)
{
    int local = 100;
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        counter += 5;
        local += 5;
        printf("child : counter=%d local=%d (&counter=%p)\n", counter, local, (void *)&counter);
        fflush(stdout);                 // _exit does NOT flush stdio: do it yourself
        _exit(0);
    }
    wait(NULL);                     // the child is finished and has already changed its copy
    printf("parent: counter=%d local=%d (&counter=%p)\n", counter, local, (void *)&counter);
    return 0;
}
