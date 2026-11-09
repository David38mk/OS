// processes.md §2 — the order of output between parent and children is not defined
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
            break;
        }
        if (p == 0) {
            printf("child %d\n", i);
            fflush(stdout);         // _exit does NOT flush stdio: do it yourself
            _exit(0);
        }
    }
    printf("parent\n");
    fflush(stdout);
    while (wait(NULL) > 0)
        ;
    return 0;
}
