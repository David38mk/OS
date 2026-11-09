// pipes.md §5 — generic "cmd1 | cmd2 > out.txt": commands come from argv split at "|"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

// usage: ./a.out ls -1 / "|" sort -r      (output goes to /tmp/10_out.txt)
int main(int argc, char *argv[]) {
    int bar = 0;
    for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "|")) bar = i;
    if (!bar) { fprintf(stderr, "need a | argument\n"); return 1; }
    argv[bar] = NULL;                       // now argv+1 and argv+bar+1 are two argv arrays

    int fd[2];
    pipe(fd);
    if (fork() == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]); close(fd[1]);
        execvp(argv[1], argv + 1);
        perror("exec 1"); _exit(127);
    }
    if (fork() == 0) {
        int out = open("/tmp/10_out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd[0], STDIN_FILENO);          // stdin <- pipe
        dup2(out, STDOUT_FILENO);           // stdout -> file
        close(fd[0]); close(fd[1]); close(out);
        execvp(argv[bar + 1], argv + bar + 1);
        perror("exec 2"); _exit(127);
    }
    close(fd[0]); close(fd[1]);
    while (wait(NULL) > 0) {}
    return 0;
}
