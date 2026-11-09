// pipes.md §2 — parent writes text into a pipe, child reads it in a loop until EOF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];                         // fd[0] = read end, fd[1] = write end
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {                    // child: reader
        close(fd[1]);                  // child never writes: close its write end
        char buf[64];
        ssize_t n;
        while ((n = read(fd[0], buf, sizeof buf - 1)) > 0) {
            buf[n] = '\0';             // read gives raw bytes, not a C string
            printf("child got %zd bytes: \"%s\"\n", n, buf);
        }
        printf("child: read returned %zd (EOF)\n", n);
        close(fd[0]);
        return 0;
    }
    close(fd[0]);                      // parent never reads: close its read end
    const char *msg = "zdravo od roditelot";
    write(fd[1], msg, strlen(msg));
    close(fd[1]);                      // last write end closed -> child sees EOF
    wait(NULL);
    return 0;
}
