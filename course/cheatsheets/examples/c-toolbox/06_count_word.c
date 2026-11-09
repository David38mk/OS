// c-toolbox.md §2 — count how often a word occurs in a file with fscanf("%255s") (2022 Z3 style)
#include <stdio.h>
#include <string.h>

#define MAX_WORD 256

int count_word(const char *file, const char *zbor, int *total) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
        return -1;                           // let the caller decide what to print
    char w[MAX_WORD];
    int found = 0;
    *total = 0;
    while (fscanf(fp, "%255s", w) == 1) {    // 255 = MAX_WORD - 1: leaves room for the '\0'
        if (strcmp(w, zbor) == 0)            // strcmp == 0 means equal
            found++;
        (*total)++;
    }
    fclose(fp);
    return found;
}

int main(void) {
    FILE *f = fopen("tekst.txt", "w");
    if (f == NULL) { perror("tekst.txt"); return 1; }
    fputs("OS e OS predmet\nna FEIT, OS e lab\n", f);
    fclose(f);
    int total;
    int n = count_word("tekst.txt", "OS", &total);
    printf("OS: %d od %d zborovi\n", n, total);
    printf("nema.txt: %d\n", count_word("nema.txt", "OS", &total));
    return 0;
}
