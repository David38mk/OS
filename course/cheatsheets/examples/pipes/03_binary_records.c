// pipes.md §3 — send binary ints and a struct (no pointers inside) through a pipe
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

struct rec { int id; double value; char name[16]; };   // fixed size, no pointers

int main(void) {
    int fd[2];
    pipe(fd);
    if (fork() == 0) {
        close(fd[1]);
        int n;
        read(fd[0], &n, sizeof n);                 // first the count ...
        printf("child: %d records\n", n);
        struct rec r;
        for (int i = 0; i < n; i++) {              // ... then exactly n records
            if (read(fd[0], &r, sizeof r) != sizeof r) { puts("short read"); break; }
            printf("child: id=%d value=%.2f name=%s\n", r.id, r.value, r.name);
        }
        close(fd[0]);
        return 0;
    }
    close(fd[0]);
    int n = 3;
    write(fd[1], &n, sizeof n);
    for (int i = 0; i < n; i++) {
        struct rec r = { i + 1, 1.5 * (i + 1), "" };
        snprintf(r.name, sizeof r.name, "item%d", i);
        write(fd[1], &r, sizeof r);
    }
    close(fd[1]);
    wait(NULL);
    return 0;
}
