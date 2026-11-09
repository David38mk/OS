// shared-memory.md §5 — 4 children increment ONE shared counter with no lock: updates get lost
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define KIDS 4
#define INCS 1000000

int main(void) {
    volatile long *counter = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE,
                                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (counter == MAP_FAILED) { perror("mmap"); return 1; }
    *counter = 0;

    for (int i = 0; i < KIDS; i++) {
        if (fork() == 0) {
            for (int k = 0; k < INCS; k++)
                (*counter)++;              // read, add, write back: three steps, not one
            _exit(0);
        }
    }
    while (wait(NULL) > 0) {}
    printf("expected %d, got %ld\n", KIDS * INCS, *counter);
    return 0;
}
