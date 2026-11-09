// c-toolbox.md §2 — file descriptors: open/write/close, then open/read loop (no FILE*, no buffering)
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    const char *tekst = "zdravo od write()\n";
    int fd = open("fd.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);   // mode only matters when creating
    if (fd < 0) { perror("open"); return 1; }
    if (write(fd, tekst, strlen(tekst)) < 0) perror("write");      // bytes, not a string: pass the length
    close(fd);

    fd = open("fd.txt", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    char buf[8];                             // small on purpose: read() returns at most 8 bytes per call
    ssize_t n;                               // signed: -1 = error, 0 = end of file
    int calls = 0;
    long total = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, n, stdout);           // buf is NOT terminated: write exactly n bytes
        total += n;
        calls++;
    }
    close(fd);
    printf("%ld bytes in %d reads\n", total, calls);
    return 0;
}
