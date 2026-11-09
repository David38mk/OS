// processes.md §9 — Lab 2 2025 Z3 done right: run the given command N times, then wait for all
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
    int N;
    printf("Vnesi kolku pati: ");
    fflush(stdout);
    if (scanf("%d", &N) != 1 || N < 1) {
        fprintf(stderr, "Nevaliden N\n");
        return 1;
    }
    for (int i = 0; i < N; i++) {
        pid_t p = fork();
        if (p < 0) {
            perror("fork");
            break;
        }
        if (p == 0) {
            execvp(argv[1], &argv[1]);
            perror(argv[1]);        // exec failed: report and leave, never fall into the loop
            _exit(127);
        }
    }
    while (wait(NULL) > 0)
        ;
    printf("Zavrshuva process 0\n");
    return 0;
}
