// processes.md §3 — run "the command given on the command line" (unknown number of args)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Upotreba: %s naredba [argumenti...]\n", argv[0]);
        return 1;
    }
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        // argv[1] is the program, &argv[1] is the array {argv[1], argv[2], ..., NULL}
        // (the C standard guarantees argv[argc] == NULL, so the terminator is already there)
        execvp(argv[1], &argv[1]);
        perror(argv[1]);            // exec returned, so it failed (e.g. command not found)
        _exit(127);
    }
    int st;
    waitpid(p, &st, 0);
    if (WIFEXITED(st))
        printf("command exited with %d\n", WEXITSTATUS(st));
    return 0;
}
