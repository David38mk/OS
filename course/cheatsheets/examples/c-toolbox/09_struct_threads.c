// c-toolbox.md §4 — a struct per thread: input fields in, result field out, array of structs, no malloc/cast tricks
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#define MAX 100
struct Zadacha {
    const char *ime;     // input: which file / currency
    double niza[MAX];    // input: the values
    int n;
    double rezultat;     // output: written by the thread, read by main after join
};
void *najgolem_rast(void *arg) {
    struct Zadacha *z = arg;                    // void * back to the real type
    z->rezultat = 0;
    for (int i = 1; i < z->n; i++)
        if (z->niza[i] - z->niza[i - 1] > z->rezultat)
            z->rezultat = z->niza[i] - z->niza[i - 1];
    return NULL;                                // result already lives in the struct: nothing to free
}
int main(void) {
    struct Zadacha z[3];
    memset(z, 0, sizeof(z));                    // n and rezultat start at 0, not at garbage
    const char *imena[3] = {"BTC", "ETH", "EOS"};
    for (int i = 0; i < 3; i++) {
        z[i].ime = imena[i];
        for (int j = 0; j < 4; j++)
            z[i].niza[z[i].n++] = (i + 1) * (j % 2 ? 10.0 * j : 3.0 * j);
    }
    pthread_t t[3];
    for (int i = 0; i < 3; i++)
        if (pthread_create(&t[i], NULL, najgolem_rast, &z[i]) != 0) {   // address of element i
            fprintf(stderr, "pthread_create failed\n");
            return 1;
        }
    for (int i = 0; i < 3; i++) {
        pthread_join(t[i], NULL);
        printf("%s: %.1f\n", z[i].ime, z[i].rezultat);
    }
    return 0;
}
