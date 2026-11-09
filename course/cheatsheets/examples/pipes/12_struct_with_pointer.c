// pipes.md §8 — writing a struct that holds a pointer sends the ADDRESS, not the text
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

struct bad { int id; char *name; };        // name is only an address

int main(void) {
    int fd[2];
    pipe(fd);
    if (fork() == 0) {
        close(fd[1]);
        struct bad b;
        read(fd[0], &b, sizeof b);
        printf("child: id=%d name pointer=%p (not dereferenced)\n", b.id, (void *)b.name);
        return 0;
    }
    close(fd[0]);
    char text[] = "Ana";
    struct bad b = { 7, text };
    printf("parent: text lives at %p\n", (void *)text);
    write(fd[1], &b, sizeof b);            // copies 16 bytes incl. the pointer value
    close(fd[1]);
    wait(NULL);
    // fix: struct { int id; char name[16]; } so the characters travel inside the struct
    return 0;
}
