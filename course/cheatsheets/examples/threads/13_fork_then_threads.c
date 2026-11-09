// threads.md §8 — parent fills arrays, child (a copy of the memory) runs the threads (2024 Z3)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define NA 3
int arr[NA][4];                     // filled by the parent BEFORE fork
int best[NA];                       // written by the child's threads

void *largest(void *arg)
{
    int a = *(int *)arg;
    best[a] = arr[a][0];
    for (int i = 1; i < 4; i++)
        if (arr[a][i] > best[a])
            best[a] = arr[a][i];
    return NULL;
}

int main(void)
{
    for (int a = 0; a < NA; a++)
        for (int i = 0; i < 4; i++)
            arr[a][i] = (a + 1) * (i + 1) % 7;
    pid_t p = fork();               // a copy of arr goes to the child
    if (p < 0) { perror("fork"); return 1; }
    if (p == 0) {                   // child: threads are created AFTER the fork
        pthread_t tid[NA];
        int idx[NA];
        for (int a = 0; a < NA; a++) {
            idx[a] = a;
            pthread_create(&tid[a], NULL, largest, &idx[a]);
        }
        for (int a = 0; a < NA; a++) {
            pthread_join(tid[a], NULL);
            printf("child : array %d largest = %d\n", a, best[a]);
        }
        fflush(stdout);
        _exit(0);
    }
    waitpid(p, NULL, 0);
    printf("parent: best[0] = %d (the child's result never came back)\n", best[0]);
    return 0;
}
