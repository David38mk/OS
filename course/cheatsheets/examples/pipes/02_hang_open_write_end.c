// pipes.md §2 — a write end left open anywhere makes read() block forever (run with timeout)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    (void)argv;
    int forget = argc > 1;             // any argument = "forget the close" (the bug)
    int fd[2];
    pipe(fd);

    if (fork() == 0) {                 // child: reader
        close(fd[1]);                  // child's own write end is closed (correct)
        char buf[64];
        ssize_t n;
        while ((n = read(fd[0], buf, sizeof buf)) > 0)
            printf("child got %zd bytes\n", n);
        printf("child: EOF, leaving\n");
        return 0;
    }
    close(fd[0]);
    write(fd[1], "hello", 5);
    if (!forget)
        close(fd[1]);                  // BUG when skipped: parent still holds a write end
    wait(NULL);                        // parent waits for a child that waits for EOF
    return 0;
}
