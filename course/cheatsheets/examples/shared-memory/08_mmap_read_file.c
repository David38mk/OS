// shared-memory.md §3 — read a whole file as a char array with mmap (count lines and spaces)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(void) {
    FILE *f = fopen("/tmp/08_text.txt", "w");          // make our own input
    fputs("eden dva tri\nchetiri pet\nshest\n", f);
    fclose(f);

    int fd = open("/tmp/08_text.txt", O_RDONLY);
    struct stat st;
    if (fd == -1 || fstat(fd, &st) == -1) { perror("open/fstat"); return 1; }
    if (st.st_size == 0) { puts("empty file"); return 0; }   // mmap of length 0 fails (EINVAL)

    char *txt = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);   // read-only is enough
    if (txt == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    int lines = 0, spaces = 0;
    for (off_t i = 0; i < st.st_size; i++) {           // index it like any char array
        if (txt[i] == '\n') lines++;
        if (txt[i] == ' ') spaces++;
    }
    printf("%lld bytes, %d lines, %d spaces\n", (long long)st.st_size, lines, spaces);
    munmap(txt, st.st_size);
    return 0;
}
