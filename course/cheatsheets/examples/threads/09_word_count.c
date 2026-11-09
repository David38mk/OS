// threads.md §7 — one thread per file counts a word; main prints percentages (2022 Z3 shape)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NF 3
const char *word = "kluc";
const char *name[NF] = {"/tmp/t09_a.txt", "/tmp/t09_b.txt", "/tmp/t09_c.txt"};
const char *text[NF] = {"kluc a kluc b kluc", "a b c", "kluc kluc x"};
int count[NF];                      // slot per thread, no lock needed

void *count_word(void *arg)
{
    int i = *(int *)arg;
    FILE *f = fopen(name[i], "r");
    if (f == NULL) { perror(name[i]); return NULL; }    // count[i] stays 0
    char w[256];
    while (fscanf(f, "%255s", w) == 1)
        if (strcmp(w, word) == 0)
            count[i]++;
    fclose(f);
    printf("%s: %d times\n", name[i], count[i]);
    return NULL;
}

int main(void)
{
    for (int i = 0; i < NF; i++) {  // create the input files for this demo
        FILE *f = fopen(name[i], "w");
        if (f == NULL) { perror(name[i]); return 1; }
        fputs(text[i], f);
        fclose(f);
    }
    pthread_t tid[NF];
    int idx[NF], total = 0;
    for (int i = 0; i < NF; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, count_word, &idx[i]);
    }
    for (int i = 0; i < NF; i++) {
        pthread_join(tid[i], NULL);
        total += count[i];
    }
    for (int i = 0; i < NF; i++)    // percentages only after ALL threads are joined
        printf("%s: %.2f%%\n", name[i], total ? 100.0 * count[i] / total : 0.0);
    return 0;
}
