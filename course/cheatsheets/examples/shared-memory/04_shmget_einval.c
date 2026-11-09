// shared-memory.md §7 — shmget with the same key but a LARGER size fails with EINVAL
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(void) {
    key_t key = ftok(".", 'E');
    int id = shmget(key, 100, IPC_CREAT | 0666);    // first creation: 100 bytes
    printf("create 100 bytes: id=%d\n", id);

    int id2 = shmget(key, 5000, IPC_CREAT | 0666);  // same key, bigger size
    if (id2 == -1)
        printf("shmget 5000 bytes: %s (errno=%d)\n", strerror(errno), errno);

    int id3 = shmget(key, 50, 0666);                // asking for LESS than the real size is allowed
    printf("shmget 50 bytes : id=%d (same segment: %s)\n", id3, id3 == id ? "yes" : "no");

    shmctl(id, IPC_RMID, NULL);                     // cleanup
    return 0;
}
