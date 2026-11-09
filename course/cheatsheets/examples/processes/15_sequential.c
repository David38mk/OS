// processes.md §8 — sequential: fork, then wait for that child before starting the next
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    for (int i = 0; i < 3; i++) {
        pid_t p = fork();
        if (p < 0) {
            perror("fork");
            return 1;
        }
        if (p == 0) {
            printf("child %d starts\n", i);
            sleep(1);
            printf("child %d ends\n", i);
            fflush(stdout);                 // _exit does NOT flush stdio: do it yourself
            _exit(0);
        }
        waitpid(p, NULL, 0);        // the next iteration starts only after this child ended
    }
    return 0;
}
