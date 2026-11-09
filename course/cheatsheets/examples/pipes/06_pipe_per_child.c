// pipes.md §3 — one pipe per child: results come back by child number; array reaches children via fork
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 3
#define LEN 5

int main(void) {
    int data[N][LEN] = { {1,2,3,4,5}, {10,20,30,40,50}, {7,7,7,7,7} };   // built BEFORE fork
    int fd[N][2];

    for (int i = 0; i < N; i++) {
        pipe(fd[i]);
        if (fork() == 0) {
            for (int j = 0; j < i; j++) close(fd[j][0]);   // older pipes: parent's read ends
            close(fd[i][0]);                               // own read end
            int sum = 0;
            for (int k = 0; k < LEN; k++) sum += data[i][k];   // child already has the copy
            write(fd[i][1], &sum, sizeof sum);
            close(fd[i][1]);
            return 0;
        }
        close(fd[i][1]);                                   // parent keeps only read ends
    }
    for (int i = 0; i < N; i++) {                          // read in child-number order
        int sum;
        read(fd[i][0], &sum, sizeof sum);
        printf("parent: row %d sums to %d\n", i, sum);
        close(fd[i][0]);
    }
    while (wait(NULL) > 0) {}
    return 0;
}
