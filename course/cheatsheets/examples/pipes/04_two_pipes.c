// pipes.md §3 — two pipes give two directions: parent sends a number, child sends back its square
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int down[2], up[2];                // down: parent -> child, up: child -> parent
    pipe(down);
    pipe(up);

    if (fork() == 0) {
        close(down[1]);                // child reads from down, writes to up
        close(up[0]);
        int x;
        read(down[0], &x, sizeof x);
        long sq = (long)x * x;
        write(up[1], &sq, sizeof sq);
        close(down[0]);
        close(up[1]);
        return 0;
    }
    close(down[0]);                    // parent writes to down, reads from up
    close(up[1]);
    int x = 12;
    write(down[1], &x, sizeof x);
    close(down[1]);
    long answer;
    if (read(up[0], &answer, sizeof answer) == sizeof answer)
        printf("parent: %d squared is %ld\n", x, answer);
    close(up[0]);
    wait(NULL);
    return 0;
}
