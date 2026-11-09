// processes.md §5 — kill(): existence check with signal 0, SIGTERM, SIGKILL
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        sleep(30);                  // a long job; _exit is never reached if we are killed
        _exit(0);
    }
    printf("alive? kill(p, 0) = %d\n", kill(p, 0));       // 0 = exists and we may signal it
    kill(p, SIGTERM);                                      // polite request to stop
    int st;
    waitpid(p, &st, 0);
    if (WIFSIGNALED(st))
        printf("child ended by signal %d (SIGTERM is %d)\n", WTERMSIG(st), SIGTERM);
    if (kill(p, 0) == -1 && errno == ESRCH)               // the pid is gone after waitpid
        printf("kill(p, 0) now fails with ESRCH: no such process\n");

    pid_t q = fork();
    if (q == 0) {
        sleep(30);
        _exit(0);
    }
    kill(q, SIGKILL);                                      // cannot be ignored or handled
    waitpid(q, &st, 0);
    printf("second child: signal %d (SIGKILL is %d)\n", WTERMSIG(st), SIGKILL);
    return 0;
}
