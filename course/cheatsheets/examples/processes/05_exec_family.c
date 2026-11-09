// processes.md §3 — the exec family: l = list of args, v = vector (array), p = search PATH
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void run(int which)
{
    pid_t p = fork();
    if (p < 0) {
        perror("fork");
        return;
    }
    if (p == 0) {
        char *v[] = {"echo", "via", which == 2 ? "execv" : "execvp", NULL};
        switch (which) {
        case 0: execl("/bin/echo", "echo", "via", "execl", (char *)NULL); break;
        case 1: execlp("echo", "echo", "via", "execlp", (char *)NULL); break;
        case 2: execv("/bin/echo", v); break;
        case 3: execvp("echo", v); break;
        }
        perror("exec");             // only reached if exec failed
        _exit(127);
    }
    waitpid(p, NULL, 0);
}

int main(void)
{
    for (int i = 0; i < 4; i++)
        run(i);
    return 0;
}
