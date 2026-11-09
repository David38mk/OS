// c-toolbox.md §3 — strcmp/strncmp/strcpy/strncpy/strcat/strlen/strstr/strchr/snprintf and buffer sizes
#include <stdio.h>
#include <string.h>

int main(void) {
    char a[16] = "OS";
    char b[16];
    strcpy(b, a);                                   // copies including '\0'; b must be big enough
    strcat(b, "-lab");                              // appends; total must still fit
    printf("%s %zu %d %d %d\n", b, strlen(b), strcmp(a, "OS"), strcmp(a, "PS") < 0, strncmp(b, "OS-x", 3));
    char s[4];
    strncpy(s, "abcdefgh", sizeof(s) - 1);          // copies at most 3 chars, and adds NO '\0' when cut
    s[sizeof(s) - 1] = '\0';                        // so always terminate it yourself
    printf("[%s]\n", s);
    const char *path = "/home/david/OS/lab.c";
    char *dot = strrchr(path, '.');                 // last '.', NULL if none
    char *hit = strstr(path, "OS");                 // first occurrence of a substring
    printf("ext=%s at=%ld slash=%s\n", dot ? dot : "-", hit ? (long)(hit - path) : -1L, strchr(path, '/') + 1);
    char buf[10], ime[] = "datoteka";
    int need = snprintf(buf, sizeof(buf), "%s:%d", ime, 42);   // never writes more than sizeof(buf)
    printf("need=%d got=[%s] truncated=%s\n", need, buf, need >= (int)sizeof(buf) ? "yes" : "no");
    char big[64];
    snprintf(big, sizeof(big), "%s_out", "in.txt");                   // the C way to build "<name>_out"
    printf("%s\n", big);
    return 0;
}
