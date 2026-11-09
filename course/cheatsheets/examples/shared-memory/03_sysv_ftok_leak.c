// shared-memory.md §2 — ftok key + shmget; pass "leak" to skip IPC_RMID and see it in ipcs -m
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char *argv[]) {
    int leak = argc > 1 && strcmp(argv[1], "leak") == 0;

    key_t key = ftok(".", 'A');            // same path + same id char => same key in every process
    if (key == -1) { perror("ftok"); return 1; }
    printf("key = 0x%x\n", (unsigned)key);

    int id = shmget(key, 4096, IPC_CREAT | 0666);   // create or open
    if (id == -1) { perror("shmget"); return 1; }

    char *mem = shmat(id, NULL, 0);
    if (mem == (void *)-1) { perror("shmat"); return 1; }
    strcpy(mem, "zdravo");
    printf("wrote \"%s\" into segment %d\n", mem, id);
    shmdt(mem);

    if (leak)
        printf("leaking on purpose: run  ipcs -m  then  ipcrm -m %d\n", id);
    else
        shmctl(id, IPC_RMID, NULL);        // mark for removal; gone once the last process detaches
    return 0;
}
