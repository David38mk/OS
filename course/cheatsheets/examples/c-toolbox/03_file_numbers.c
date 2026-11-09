// c-toolbox.md §2 — read all reals from a file into an array with fscanf("%lf"), biggest rise (2024 Z3)
#include <stdio.h>

#define MAX 100

int main(void) {
    FILE *w = fopen("btc.txt", "w");         // create our own input
    if (w == NULL) { perror("btc.txt"); return 1; }
    fprintf(w, "100.5 90.25 150\n120.75\n");
    fclose(w);

    FILE *fp = fopen("btc.txt", "r");
    if (fp == NULL) { perror("btc.txt"); return 1; }
    double niza[MAX];
    int n = 0;
    while (n < MAX && fscanf(fp, "%lf", &niza[n]) == 1)   // 1 = one value converted; EOF/garbage stops
        n++;
    fclose(fp);

    double max = 0;
    for (int i = 1; i < n; i++)
        if (niza[i] - niza[i - 1] > max)
            max = niza[i] - niza[i - 1];
    printf("n=%d najgolemo zgolemuvanje=%.2f\n", n, max);
    return 0;
}
