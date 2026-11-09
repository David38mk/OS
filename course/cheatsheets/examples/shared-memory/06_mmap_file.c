// shared-memory.md §3 — file-backed mmap: open, ftruncate, mmap, write, msync, munmap, close
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define SIZE 4096

int main(void) {
    int fd = open("/tmp/06_map.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) { perror("open"); return 1; }
    if (ftruncate(fd, SIZE) == -1) { perror("ftruncate"); return 1; }   // file must be big enough

    char *mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) { perror("mmap"); return 1; }

    strcpy(mem, "tekst napisan so obicna dodelba");   // plain memory write, no write() call
    mem[0] = 'T';
    msync(mem, SIZE, MS_SYNC);                        // force it to the file now (optional)

    munmap(mem, SIZE);
    close(fd);                                        // mapping stays valid after close, but we are done

    char line[40] = "";                               // prove it: read the file the normal way
    fd = open("/tmp/06_map.dat", O_RDONLY);
    read(fd, line, sizeof line - 1);
    close(fd);
    printf("file now starts with: \"%s\"\n", line);
    return 0;
}
