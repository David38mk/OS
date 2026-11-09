// processes.md §4 — waitpid and the status macros: exit code vs killed by a signal
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t kid[3];
    for (int i = 0; i < 3; i++) {
        kid[i] = fork();
        if (kid[i] < 0) {
            perror("fork");
            return 1;
        }
        if (kid[i] == 0) {
            if (i == 0) _exit(0);           // success
            if (i == 1) _exit(300);         // only the low 8 bits survive: 300 & 255 = 44
            raise(SIGKILL);                 // dies from a signal, never reaches an exit code
            _exit(0);
        }
    }
    for (int i = 0; i < 3; i++) {           // wait for each child by its own pid
        int st;
        waitpid(kid[i], &st, 0);
        if (WIFEXITED(st))
            printf("child %d: exited, code %d\n", i, WEXITSTATUS(st));
        else if (WIFSIGNALED(st))
            printf("child %d: killed by signal %d\n", i, WTERMSIG(st));
    }
    return 0;
}
