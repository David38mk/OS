// processes.md §7 — buffered stdout is copied by fork: the text is printed twice when piped
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int flush = (argc > 1 && strcmp(argv[1], "flush") == 0);

    printf("Vnesi N: ");              // no '\n': stays in the buffer when stdout is a pipe/file
    if (flush)
        fflush(stdout);             // the fix: empty the buffer BEFORE fork
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        printf("[child] ");
        exit(0);                    // exit() flushes the child's copy of the buffer
    }
    wait(NULL);
    printf("[parent]\n");
    return 0;
}
