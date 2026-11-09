// c-toolbox.md §3 — strtok (modifies the string, keeps state), strtok_r, sscanf for a fixed shape
#include <stdio.h>
#include <string.h>

int main(void) {
    char original[] = "ls -l  -a\tfile.txt";   // an ARRAY: writable. char *p = "..." would crash strtok
    char copy[sizeof(original)];
    strcpy(copy, original);                  // strtok writes '\0' into the string: work on a copy

    for (char *t = strtok(copy, " \t"); t != NULL; t = strtok(NULL, " \t"))   // NULL = continue same string
        printf("token [%s]\n", t);           // runs of separators give no empty tokens
    printf("copy is now \"%s\" (cut at the first token)\n", copy);
    printf("original is still \"%s\"\n", original);

    strcpy(copy, original);
    char *save;                              // strtok_r keeps its position here, not in a hidden global
    int n = 0;
    for (char *t = strtok_r(copy, " \t", &save); t; t = strtok_r(NULL, " \t", &save))
        n++;
    printf("strtok_r counted %d tokens\n", n);

    int a, b, c, d;
    if (sscanf("192.168.1.20", "%d.%d.%d.%d", &a, &b, &c, &d) == 4)   // returns how many were filled
        printf("ip parts %d %d %d %d\n", a, b, c, d);
    printf("sscanf of bad text returns %d\n", sscanf("1.2.x", "%d.%d.%d.%d", &a, &b, &c, &d));
    return 0;
}
