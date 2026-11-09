// c-toolbox.md §8 — stdout buffering: output order when piped, and duplicate output after fork
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    printf("1. stdout, ima newline\n");             // on a terminal: line buffered. In a pipe: buffered until exit
    fprintf(stderr, "2. stderr, bez baferiranje\n");// stderr is never buffered: appears at once
    printf("3. bez newline i bez fflush ");         // sits in the buffer
    pid_t p = fork();                               // the child gets a COPY of the unflushed buffer
    if (p == 0) {
        printf("(dete)\n");
        return 0;                                   // exit flushes the child's copy
    }
    wait(NULL);
    fflush(stdout);                                 // fflush(stdout) forces it out now
    printf("4. kraj\n");
    return 0;
}
