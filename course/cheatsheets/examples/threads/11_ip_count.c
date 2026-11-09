// threads.md §7 — one thread per IP counts addresses up to 255.255.255.255; main sums (2023 Z3)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

char ip[3][16] = {"255.255.255.250", "255.255.254.0", "255.255.255.255"};

void *count_ips(void *arg)
{
    int i = *(int *)arg;
    int p[4];
    long long *res = malloc(sizeof *res);
    if (res == NULL)
        return NULL;
    *res = 0;
    if (sscanf(ip[i], "%d.%d.%d.%d", &p[0], &p[1], &p[2], &p[3]) != 4)
        return res;                 // bad line: contributes 0
    // the address is a base-256 number; IPs from here up to 255.255.255.255 inclusive:
    long long here = ((long long)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    *res = 0xFFFFFFFFLL - here + 1;
    return res;
}

int main(void)
{
    pthread_t tid[3];
    int idx[3];
    for (int i = 0; i < 3; i++) {
        idx[i] = i;
        pthread_create(&tid[i], NULL, count_ips, &idx[i]);
    }
    long long total = 0;
    for (int i = 0; i < 3; i++) {
        void *ret;
        pthread_join(tid[i], &ret);
        if (ret != NULL) {
            printf("%s -> %lld\n", ip[i], *(long long *)ret);
            total += *(long long *)ret;
            free(ret);
        }
    }
    printf("Vkupno mogni IP adresi: %lld\n", total);
    return 0;
}
