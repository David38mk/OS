// threads.md §10 — strtok keeps hidden global state; in threads use strtok_r with a private state
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int words[2];

void *count_words(void *arg)
{
    int i = *(int *)arg;
    char line[64];                  // strtok modifies its input: work on a private copy
    strcpy(line, i == 0 ? "a b c d e f" : "x y");
    char *save;                     // strtok_r keeps its state here, not in a global
    for (char *w = strtok_r(line, " ", &save); w != NULL; w = strtok_r(NULL, " ", &save))
        words[i]++;
    return NULL;
}

int main(void)
{
    pthread_t tid[2];
    int idx[2] = {0, 1};
    for (int i = 0; i < 2; i++)
        pthread_create(&tid[i], NULL, count_words, &idx[i]);
    for (int i = 0; i < 2; i++)
        pthread_join(tid[i], NULL);
    printf("thread 0 counted %d words (expected 6), thread 1 counted %d (expected 2)\n", words[0], words[1]);
    return 0;
}
