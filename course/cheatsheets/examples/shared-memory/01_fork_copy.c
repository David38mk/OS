// shared-memory.md §1 — fork copies memory: the child's changes are invisible to the parent
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int counter = 10;                          // global, also copied by fork

int main(void) {
    int local = 1;
    pid_t pid = fork();
    if (pid == 0) {
        counter += 100;                    // changes the CHILD's private copy only
        local += 100;
        printf("child : counter=%d local=%d\n", counter, local);
        return 0;
    }
    wait(NULL);                            // child is certainly finished here
    printf("parent: counter=%d local=%d   (unchanged)\n", counter, local);
    return 0;
}
