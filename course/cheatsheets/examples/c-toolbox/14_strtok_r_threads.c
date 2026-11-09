// c-toolbox.md §3 — threads that tokenise: strtok_r with a private copy and private state per thread
#include <stdio.h>
#include <string.h>
#include <pthread.h>

struct Linija {
    char tekst[64];                             // each thread owns its copy: strtok_r writes into it
    int zborovi;
};

void *broi(void *arg) {
    struct Linija *l = arg;
    char *save;                                 // private position: two threads cannot disturb each other
    l->zborovi = 0;
    for (char *t = strtok_r(l->tekst, " ", &save); t != NULL; t = strtok_r(NULL, " ", &save))
        l->zborovi++;
    return NULL;
}

int main(void) {
    struct Linija l[2];
    strcpy(l[0].tekst, "eden dva tri");
    strcpy(l[1].tekst, "a b c d e f");
    pthread_t t[2];
    for (int i = 0; i < 2; i++)
        pthread_create(&t[i], NULL, broi, &l[i]);
    for (int i = 0; i < 2; i++) {
        pthread_join(t[i], NULL);
        printf("linija %d: %d zborovi\n", i, l[i].zborovi);
    }
    return 0;
}
