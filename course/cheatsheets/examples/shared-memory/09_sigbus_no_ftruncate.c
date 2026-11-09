// shared-memory.md §7 — mmap on an empty file WITHOUT ftruncate: touching the memory gives SIGBUS
// expect: signal 7  (SIGBUS is the point of this example; check.sh reads this line)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(void) {
    int fd = open("/tmp/09_empty.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);   // size 0
    // ftruncate(fd, 4096);                  // <-- the missing line
    char *mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) { perror("mmap"); return 1; }   // mmap itself succeeds!
    printf("mmap ok at %p, writing now...\n", (void *)mem);
    fflush(stdout);
    mem[0] = 'x';                            // no file page behind this address -> SIGBUS
    printf("not reached\n");
    return 0;
}
