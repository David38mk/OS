// processes.md §2 — fork in a loop: correct (child exits) vs the 2^N mistake
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int N = 3;
    int bad = (argc > 1 && strcmp(argv[1], "bad") == 0);

    for (int i = 0; i < N; i++) {
        pid_t p = fork();
        if (p < 0) {
            perror("fork");
            break;
        }
        if (p == 0) {
            printf("child %d from iteration %d\n", getpid(), i);
            fflush(stdout);
            if (!bad)
                _exit(0);           // correct: the child leaves the loop at once
            // bad: the child keeps looping and forks too, so there are 2^N children in total
        }
    }
    while (wait(NULL) > 0)          // each process waits for its own children
        ;
    return 0;
}
