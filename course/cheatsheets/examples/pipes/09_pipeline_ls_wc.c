// pipes.md §5 — build "ls -1 | wc -l" by hand: two forks, dup2 on each side, close everything
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    pid_t p1 = fork();
    if (p1 == 0) {                          // left side: stdout -> pipe
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]); close(fd[1]);         // both originals, or wc never sees EOF
        execlp("ls", "ls", "-1", (char *)NULL);
        perror("execlp ls"); _exit(127);
    }
    pid_t p2 = fork();
    if (p2 == 0) {                          // right side: stdin <- pipe
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]); close(fd[1]);
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("execlp wc"); _exit(127);
    }
    close(fd[0]); close(fd[1]);             // the parent holds both ends too: close them
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    return 0;
}
