// pipes.md §4 — dup2 onto fd 1 and fd 0: printf in the parent arrives in the child's scanf
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    pipe(fd);

    if (fork() == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);         // fd 0 now IS the read end
        close(fd[0]);                      // original no longer needed
        int a, b;
        scanf("%d %d", &a, &b);            // reads from the pipe
        printf("child: %d + %d = %d\n", a, b, a + b);
        return 0;
    }
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);            // fd 1 now IS the write end
    close(fd[1]);
    printf("20 22\n");                     // goes into the pipe, not the screen
    fflush(stdout);                        // stdout is a pipe now: fully buffered, so flush
    close(STDOUT_FILENO);                  // last write end gone -> child sees EOF
    wait(NULL);
    return 0;
}
