// c-toolbox.md §8 — pitfalls shown safely: == on strings, int vs size_t, uninitialised counter, local array
#include <stdio.h>
#include <string.h>

const char *ime_lokalno(void) {
    static char buf[16];                            // static lives after return; a plain local array would not
    strcpy(buf, "lokalno");
    return buf;
}

int main(int argc, char *argv[]) {
    char a[] = "OS";
    char b[] = "OS";
    const char *pa = a, *pb = b;                    // a == b written directly: gcc warns (-Warray-compare)
    printf("pa == pb: %d   strcmp == 0: %d\n", pa == pb, strcmp(a, b) == 0);   // == compares addresses
    const char *lit = "-r";                         // argv[1] == "-r" written directly: gcc warns (-Waddress)
    if (argc > 1)
        printf("argv[1] == lit: %d   strcmp: %d\n", argv[1] == lit, strcmp(argv[1], lit) == 0);

    int n = -1;
    size_t len = strlen(a);                         // unsigned
    printf("-1 < 2u: %d   (int)len > n: %d\n", n < (int)len, (int)len > n);
    printf("-1 < size_t: %d  (the -1 became a huge unsigned number)\n", (size_t)n < len);

    int brojac = 0;                                 // always give counters a starting value
    for (int i = 0; i < 3; i++)
        brojac++;
    printf("brojac=%d, text from a function: %s\n", brojac, ime_lokalno());
    return 0;
}
