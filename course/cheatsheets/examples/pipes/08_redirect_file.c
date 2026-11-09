// pipes.md §4 — stdout to a file (O_TRUNC) and stdin from a file, with dup2
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    FILE *f = fopen("/tmp/08_in.txt", "w");        // make our own input file
    fprintf(f, "alfa 42\n");
    fclose(f);

    int in = open("/tmp/08_in.txt", O_RDONLY);
    int out = open("/tmp/08_out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (in == -1 || out == -1) { perror("open"); return 1; }

    int screen = dup(STDOUT_FILENO);               // keep the real stdout
    dup2(in, STDIN_FILENO);   close(in);           // like:  < /tmp/08_in.txt
    dup2(out, STDOUT_FILENO); close(out);          // like:  > /tmp/08_out.txt

    char word[32];
    int n;
    scanf("%31s %d", word, &n);
    printf("word=%s twice=%d\n", word, 2 * n);     // lands in the file
    fflush(stdout);                                // before restoring or exiting

    dup2(screen, STDOUT_FILENO);                   // back to the screen
    printf("done, now run: cat /tmp/08_out.txt\n");
    return 0;
}
