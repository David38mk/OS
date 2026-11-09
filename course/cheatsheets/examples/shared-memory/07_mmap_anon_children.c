// shared-memory.md §3 — anonymous shared mmap (no file, no key): children report into a shared array
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define N 4

int main(void) {
    int *res = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);   // fd = -1: no file behind it
    if (res == MAP_FAILED) { perror("mmap"); return 1; }

    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            res[i] = (i + 1) * (i + 1);                   // child i fills slot i
            return 0;
        }
    }
    while (wait(NULL) > 0) {}
    for (int i = 0; i < N; i++) printf("slot %d = %d\n", i, res[i]);
    munmap(res, N * sizeof(int));                         // vanishes with the last process anyway
    return 0;
}
