// c-toolbox.md §2 — read lines with fgets, strip the newline with strcspn, long lines, last line without \n
#include <stdio.h>
#include <string.h>

int main(void) {
    FILE *w = fopen("lines.txt", "w");
    if (w == NULL) { perror("lines.txt"); return 1; }
    fputs("10.0.0.1\n\n192.168.0.255\nposledna bez newline", w);
    fclose(w);

    FILE *fp = fopen("lines.txt", "r");
    if (fp == NULL) { perror("lines.txt"); return 1; }
    char line[16];                           // deliberately small: a longer line arrives in pieces
    int nr = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {   // NULL at end of file
        int had_nl = strchr(line, '\n') != NULL;
        line[strcspn(line, "\n")] = '\0';    // index of the first '\n' (or the end): cut there
        nr++;
        printf("%d: [%s] len=%zu %s\n", nr, line, strlen(line), had_nl ? "" : "(no newline: partial or last)");
    }
    fclose(fp);
    return 0;
}
