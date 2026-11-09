// c-toolbox.md §1 — argc/argv, usage message + exit code, strtol with error detection vs atoi
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {                          // argv[0] is the program name, so 2 = one real argument
        fprintf(stderr, "Upotreba: %s <N>...\n", argv[0]);
        return 1;                            // non-zero = failure, the shell sees it in $?
    }
    for (int i = 1; i < argc; i++) {
        char *end;                           // strtol stores here where it stopped reading
        errno = 0;
        long v = strtol(argv[i], &end, 10);
        int ok = *argv[i] != '\0'            // something was given
              && *end == '\0'                // the whole text was a number
              && errno == 0                  // no overflow
              && v >= 1 && v <= 1000;        // our own range
        printf("%-22s atoi=%-11d strtol=%-20ld %s\n", argv[i], atoi(argv[i]), v,
               ok ? "OK" : "INVALID");
    }
    return 0;
}
