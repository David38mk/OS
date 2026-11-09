// shared-memory.md §2 — System V shm with IPC_PRIVATE: child fills a struct, parent waits, reads, removes
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

struct shared { int count; int values[5]; char text[32]; };   // plain data, no pointers

int main(void) {
    int id = shmget(IPC_PRIVATE, sizeof(struct shared), IPC_CREAT | 0666);
    if (id == -1) { perror("shmget"); return 1; }

    pid_t pid = fork();                    // each process attaches the segment itself
    if (pid == 0) {
        struct shared *s = shmat(id, NULL, 0);
        if (s == (void *)-1) { perror("child shmat"); return 1; }
        s->count = 5;
        for (int i = 0; i < 5; i++) s->values[i] = (i + 1) * 11;
        strcpy(s->text, "napisano od deteto");
        shmdt(s);                          // detach: this process no longer sees it
        return 0;
    }
    wait(NULL);                            // sequencing = synchronisation: child is done
    struct shared *s = shmat(id, NULL, 0);
    if (s == (void *)-1) { perror("parent shmat"); return 1; }
    printf("parent reads: count=%d text=\"%s\" values:", s->count, s->text);
    for (int i = 0; i < s->count; i++) printf(" %d", s->values[i]);
    printf("\n");
    shmdt(s);
    shmctl(id, IPC_RMID, NULL);            // parent removes the segment: nobody else needs it
    return 0;
}
