// shared-memory.md §7 — a pointer stored inside shared memory is only valid in the mapping that made it
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct node { int value; struct node *next; int next_idx; };   // pointer vs index

int main(void) {
    int id = shmget(IPC_PRIVATE, 2 * sizeof(struct node), IPC_CREAT | 0600);
    struct node *a = shmat(id, NULL, 0);   // attachment #1
    struct node *b = shmat(id, NULL, 0);   // attachment #2 of the SAME memory (like another process)
    printf("attach #1 at %p, attach #2 at %p\n", (void *)a, (void *)b);

    a[0].value = 1;  a[1].value = 2;
    a[0].next = &a[1];                     // pointer: an address inside mapping #1
    a[0].next_idx = 1;                     // index: means "element 1" in ANY mapping

    struct node *viaptr = b[0].next;       // reader using mapping #2 sees mapping #1's address
    int inside = (char *)viaptr >= (char *)b && (char *)viaptr < (char *)b + 2 * sizeof(struct node);
    printf("pointer %p is inside mapping #2? %s\n", (void *)viaptr, inside ? "yes" : "NO");
    printf("index   %d -> b[%d].value = %d  (always right)\n",
           b[0].next_idx, b[0].next_idx, b[b[0].next_idx].value);

    shmdt(a); shmdt(b);
    shmctl(id, IPC_RMID, NULL);
    return 0;
}
