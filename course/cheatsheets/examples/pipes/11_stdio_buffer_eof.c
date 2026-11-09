// pipes.md §8 — stdin redirected twice: stdio buffer and sticky EOF (the 2020 Z3_2 shape)
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    FILE *f = fopen("/tmp/11_dat.txt", "w");
    fputs("beta gama\n", f);
    fclose(f);

    int in = open("/tmp/11_dat.txt", O_RDONLY);
    char a[32] = "", b[32] = "";
    int r1, r2;

    r1 = scanf("%31s", a);                 // stdin = whatever the shell gave us (a pipe here)
    dup2(in, STDIN_FILENO);                // fd 0 is now the file ...
    r2 = scanf("%31s", b);                 // ... but stdio's FILE may still be at EOF
    printf("first scanf=%d (%s), second scanf=%d (%s)\n", r1, a, r2, b);
    return 0;
}
