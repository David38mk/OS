// БОНУС2: дали чекањето на дете процесите може да се направи со нитки, по една нитка за секое дете, наместо со
// една јамка во родителот? Што се добива, а што се губи со тој пристап?
//
// Одговор:
//  - Нитките МОЖАТ да го направат чекањето (тие се во родителот, па waitpid работи од секоја нитка).
//  - Нитките НЕ МОЖАТ да ги заменат процесите: exec го заменува целиот процес, заедно со сите негови нитки.
//  - Затоа: прво fork на сите деца, дури потоа креирање на нитките.
//
// Добивка: блокирачко чекање (waitpid без WNOHANG, нема будење на секои 100 ms), нема евиденција за
// "кое дете сѐ уште работи" во јамка, еден тајмер во main. Загуба: заедничката состојба бара mutex, правилото
// fork-пред-нитки (fork во повеќенитна програма е ризичен), и повеќе код за истиот резултат.
//
// Употреба: ./Z2_threads host...   (без -r)
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

struct Job {
    pid_t pid;
    const char *host;
    double start;
    const char *status;
    int secs;
    int done;          // se menuva pod mutex
};

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Monoton chasovnik so delovi od sekunda (vidi komentar vo Z2.c: time() ima
// rezolucija od 1 s i bi dalo predvremen timeout).
static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}

static void *waiter(void *arg)
{
    struct Job *j = arg;

    // Chekor 1: blokirachko chekanje BEZ zhneenje (WNOWAIT). Deteto ostanuva
    // zombi dodeka ne go zhneeme pod kluch, pa negoviot pid ne mozhe da bide
    // daden na drug proces vo megjuvreme.
    siginfo_t si;
    while (waitid(P_PID, j->pid, &si, WEXITED | WNOWAIT) == -1 && errno == EINTR)
        ;
    double secs = now_s() - j->start;

    // Chekor 2: zhneenje i oznaka done pod istiot kluch pod koj main prakja
    // kill. Taka kill nikogash ne pogoduva tugj (recikliran) pid: ili done==0
    // i pid-ot e seushte nasheto dete (zhivo ili zombi), ili done==1 i main go
    // preskoknuva.
    pthread_mutex_lock(&lock);
    int st = 0;
    waitpid(j->pid, &st, 0);   // zavrshuva vednash, deteto vekje e gotovo
    if (WIFSIGNALED(st) && WTERMSIG(st) == SIGKILL)
        j->status = "TIMEOUT";
    else if (WIFEXITED(st) && WEXITSTATUS(st) == 0)
        j->status = "UP";
    else
        j->status = "DOWN";
    j->secs = (int)secs;
    j->done = 1;
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Upotreba: %s host...\n", argv[0]);
        return 1;
    }
    int n = argc - 1;

    int T;
    printf("Vnesi timeout vo sekundi: ");
    fflush(stdout);
    if (scanf("%d", &T) != 1 || T < 1) {
        fprintf(stderr, "Greshka: timeout mora da bide cel broj >= 1\n");
        return 1;
    }

    struct Job jobs[n];
    pthread_t tid[n];

    // Prvo fork na site deca (pred nitkite).
    for (int i = 0; i < n; i++) {
        jobs[i].host = argv[i + 1];
        jobs[i].status = "DOWN";
        jobs[i].secs = 0;
        jobs[i].done = 0;
        fflush(stdout);   // inaku deteto go nasleduva nepraznoto kopie na baferot
        jobs[i].pid = fork();
        if (jobs[i].pid < 0) {
            perror("fork");
            jobs[i].done = 1;
            continue;
        }
        if (jobs[i].pid == 0) {
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
            execlp("ping", "ping", "-c", "1", jobs[i].host, (char *)NULL);
            _exit(127);   // _exit, ne exit: ne gi prazni baferite na roditelot
        }
        jobs[i].start = now_s();
    }

    int started[n];
    for (int i = 0; i < n; i++) {
        started[i] = 0;
        if (jobs[i].done)
            continue;
        if (pthread_create(&tid[i], NULL, waiter, &jobs[i]) == 0) {
            started[i] = 1;
        } else {
            perror("pthread_create");
            kill(jobs[i].pid, SIGKILL);
            waitpid(jobs[i].pid, NULL, 0);
            jobs[i].done = 1;   // inaku main bi chekal T sekundi i bi pratil kill na zhneen pid
        }
    }

    // Eden tajmer: do T sekundi, proverka na sekoi 100 ms dali site se gotovi.
    for (int t = 0; t < T * 10; t++) {
        struct timespec ts = {0, 100000000L};
        nanosleep(&ts, NULL);
        int all = 1;
        pthread_mutex_lock(&lock);
        for (int i = 0; i < n; i++)
            if (!jobs[i].done)
                all = 0;
        pthread_mutex_unlock(&lock);
        if (all)
            break;
    }
    // Vremeto istece: ubij gi onie shto ne zavrshile (nitkata ke go zhnee so waitpid).
    pthread_mutex_lock(&lock);
    for (int i = 0; i < n; i++)
        if (!jobs[i].done)
            kill(jobs[i].pid, SIGKILL);
    pthread_mutex_unlock(&lock);

    for (int i = 0; i < n; i++)
        if (started[i])
            pthread_join(tid[i], NULL);

    for (int i = 0; i < n; i++) {
        // TIMEOUT se prikazhuva so T sekundi (kako vo Z2.c)
        int secs = (jobs[i].status[0] == 'T') ? T : jobs[i].secs;
        printf("%s %s (%d s)\n", jobs[i].host, jobs[i].status, secs);
    }
    printf("Zavrshuva process 0\n");
    return 0;
}
