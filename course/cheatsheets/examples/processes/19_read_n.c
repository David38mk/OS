// processes.md §8 — parent reads N from the keyboard, then forks N children (Lab 2 Z3 shape)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int N;
    printf("Vnesi N: ");
    fflush(stdout);                         // prompt out BEFORE any fork
    if (scanf("%d", &N) != 1 || N < 1 || N > 100) {   // check before using N as an array size
        fprintf(stderr, "N must be 1..100\n");
        return 1;
    }
    pid_t kid[N];                           // a VLA is fine once N is validated
    for (int i = 0; i < N; i++) {
        kid[i] = fork();
        if (kid[i] < 0) {
            perror("fork");
            return 1;
        }
        if (kid[i] == 0) {
            printf("child %d (pid %d)\n", i, getpid());
            fflush(stdout);                 // _exit does NOT flush stdio: do it yourself
            _exit(0);
        }
    }
    for (int i = 0; i < N; i++)
        waitpid(kid[i], NULL, 0);
    printf("Zavrshuva process 0\n");
    return 0;
}
