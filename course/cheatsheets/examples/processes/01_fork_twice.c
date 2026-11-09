// processes.md §1 — fork() returns twice: once in the parent, once in the child
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    printf("before fork: pid=%d ppid=%d\n", getpid(), getppid());
    fflush(stdout);                 // empty the buffer so it is not copied (section 7)

    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        printf("CHILD : fork returned %d, pid=%d ppid=%d\n", p, getpid(), getppid());
        fflush(stdout);                 // _exit does NOT flush stdio: do it yourself
        _exit(0);
    }
    printf("PARENT: fork returned %d (the child's pid), pid=%d\n", p, getpid());
    wait(NULL);
    return 0;
}
