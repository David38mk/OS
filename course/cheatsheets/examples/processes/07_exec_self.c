// processes.md §3 — fork + exec of another program in the same directory (Lab 3 shape)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "second") == 0) {   // this is "program 2"
        printf("program 2: started with arg '%s', my parent is %d\n", argv[2], getppid());
        return 0;
    }
    char num[16];
    snprintf(num, sizeof num, "%d", 42);
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return 1;
    }
    if (p == 0) {
        // in the lab this is execl("./lab31", "lab31", argv[1], argv[2], NULL);
        // here the program starts itself, so the example needs only one file
        execl(argv[0], "07_exec_self", "second", num, (char *)NULL);
        perror("execl");
        _exit(127);
    }
    printf("program 1: my pid is %d, child is %d\n", getpid(), p);
    waitpid(p, NULL, 0);
    return 0;
}
