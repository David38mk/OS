// shared-memory.md §7 — MAP_PRIVATE is copy-on-write: the child's write never reaches the parent
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

static void run(int flags, const char *name) {
    int *p = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, flags | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("mmap"); return; }
    *p = 1;
    if (fork() == 0) { *p = 99; _exit(0); }
    wait(NULL);
    printf("%-11s parent sees %d\n", name, *p);
    munmap(p, sizeof(int));
}

int main(void) {
    run(MAP_SHARED, "MAP_SHARED");         // prints 99
    run(MAP_PRIVATE, "MAP_PRIVATE");       // prints 1: no error, no warning, just a private copy
    return 0;
}
