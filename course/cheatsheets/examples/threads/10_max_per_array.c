// threads.md §7 — one thread per array finds its largest jump between neighbours (2024 Z3 shape)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NA 3
#define LEN 5

float arr[NA][LEN] = {              // declared BEFORE the thread function that uses it
    {1.0f, 2.5f, 2.0f, 6.0f, 5.0f},
    {10.0f, 9.0f, 8.0f, 7.0f, 6.0f},
    {0.5f, 0.6f, 3.6f, 3.7f, 3.8f},
};
const char *name[NA] = {"BTC", "ETH", "EOS"};

void *max_rise(void *arg)
{
    int a = *(int *)arg;            // an index, passed by ADDRESS (not cast to void*)
    float *res = malloc(sizeof *res);
    if (res == NULL)
        return NULL;
    *res = 0.0f;
    for (int i = 1; i < LEN; i++)
        if (arr[a][i] - arr[a][i - 1] > *res)
            *res = arr[a][i] - arr[a][i - 1];
    return res;                     // heap memory: still valid after the thread is gone
}

int main(void)
{
    pthread_t tid[NA];
    int idx[NA];
    for (int i = 0; i < NA; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, max_rise, &idx[i]);
    }
    for (int i = 0; i < NA; i++) {
        void *ret;
        pthread_join(tid[i], &ret);
        float r = ret ? *(float *)ret : 0.0f;
        free(ret);
        printf("Najgolemo zgolemuvanje za %s e: %.2f\n", name[i], r);
    }
    return 0;
}
