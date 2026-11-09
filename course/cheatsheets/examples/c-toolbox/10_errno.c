// c-toolbox.md §5 — errno, perror, strerror; open/fopen set errno, pthread_create RETURNS the error code
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
void *nitka(void *arg) { return arg; }
int main(void) {
    FILE *fp = fopen("/nema/takva/datoteka", "r");
    if (fp == NULL) {
        int e = errno;                              // save it at once: perror and printf may change errno
        perror("fopen");                            // "fopen: No such file or directory" on stderr
        printf("errno=%d (%s)\n", e, strerror(e));
    }
    int fd = open("/etc/shadow", O_RDONLY);         // exists, but a normal user may not read it
    if (fd < 0)
        fprintf(stderr, "open: %s\n", strerror(errno));
    FILE *ok = fopen("/etc/passwd", "r");           // success does NOT reset errno: test the return value first
    printf("errno after a success: %d\n", errno);
    if (ok != NULL)
        fclose(ok);
    pthread_t t;
    int rc = pthread_create(&t, NULL, nitka, NULL);
    if (rc != 0) {                                  // pthread_* do not use errno: the code is the return value
        fprintf(stderr, "pthread_create: %s\n", strerror(rc));
        return 1;
    }
    pthread_join(t, NULL);
    pid_t p = fork();                               // -1 on failure, 0 in the child, pid in the parent
    if (p < 0) { perror("fork"); return 1; }
    if (p == 0)
        _exit(7);                                   // child: leave with an exit status
    int st;
    wait(&st);
    printf("child status %d\n", WIFEXITED(st) ? WEXITSTATUS(st) : -1);
    return 0;
}
