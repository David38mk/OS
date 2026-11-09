// processes.md §4 — zombie: the child has exited but the parent has not called wait yet
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void show(pid_t p)
{
    char cmd[96];
    snprintf(cmd, sizeof cmd, "ps -o pid,ppid,stat,comm -p %d || echo 'pid %d: gone'", p, p);
    fflush(stdout);                 // flush before system() starts another process
    if (system(cmd) < 0)
        perror("system");
}

int main(void)
{
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0)
        _exit(0);                   // the child ends immediately
    sleep(1);                       // the parent is "busy" and does not wait
    printf("before wait():\n");
    show(p);                        // STAT is Z, comm is <defunct>
    wait(NULL);                     // the parent collects the exit status: the zombie disappears
    printf("after wait():\n");
    show(p);
    return 0;
}
