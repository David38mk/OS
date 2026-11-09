// shared-memory.md §6 — "N children, parent needs all results": shm int array indexed by child number
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define N 4

int main(void) {
    int id = shmget(IPC_PRIVATE, N * sizeof(long), IPC_CREAT | 0600);
    if (id == -1) { perror("shmget"); return 1; }
    long *res = shmat(id, NULL, 0);                 // attach BEFORE fork: children inherit it
    if (res == (void *)-1) { perror("shmat"); return 1; }

    for (int i = 0; i < N; i++) {
        if (fork() == 0) {
            long sum = 0;
            for (int k = 1; k <= (i + 1) * 100; k++) sum += k;
            res[i] = sum;                           // own slot: nobody else writes here
            shmdt(res);
            return 0;
        }
    }
    while (wait(NULL) > 0) {}                       // all children finished: safe to read
    for (int i = 0; i < N; i++) printf("child %d -> %ld\n", i, res[i]);

    shmdt(res);
    shmctl(id, IPC_RMID, NULL);
    return 0;
}
