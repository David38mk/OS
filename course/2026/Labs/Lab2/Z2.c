// Задача 2: Да се напише C програма која проверува дали повеќе хостови (сервери) се достапни, истовремено.
// Хостовите (IP адреси или имиња) се внесуваат како аргументи од командна линија и нивниот број е променлив.
// Доколку програмата се повика без аргументи, да заврши со порака за грешка.
// Родител процесот најпрво од тастатура вчитува колку секунди (T) најмногу смее да трае една проверка.
// Потоа за секој хост креира по едно дете, а секое дете ја извршува наредбата `ping -c 1 <хост>`. Родителот не
// чека бесконечно: доколку некое дете не заврши за T секунди, родителот го прекинува (убива).
// Кога ќе завршат сите деца, родителот за секој хост, во редоследот во кој се наведени, печати една линија со
// резултат и со времето во секунди колку траела проверката:
//   UP      - доколку наредбата завршила успешно (излезен статус 0),
//   DOWN    - доколку наредбата завршила со грешка (излезен статус различен од 0),
//   TIMEOUT - доколку родителот морал да го прекине детето.
// На крај родителот ја испишува пораката: "Zavrshuva process 0".
//
// БОНУС1: доколку првиот аргумент е `-r` проследен со број M, тогаш секој хост што е DOWN се проверува повторно,
// најмногу M пати, со пауза од една секунда меѓу обидите. Во резултатот за тој хост дополнително се печати по
// колку обиди станал UP, или дека останал DOWN по M обиди.
//
// Употреба: ./Z2 [-r M] host...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

struct Result {
    const char *host;
    const char *status;
    int secs;
    int attempts;
};

// Sekundi od monoton chasovnik, so delovi od sekunda. time() ne e dobro za
// merenje na T: ima rezolucija od 1 s, pa "pominaa 2 s" bi vazhelo vekje po
// 1.01 s realno vreme i deteto bi bilo ubieno predvreme. Monotoniot chasovnik
// ne skoka ni koga sistemot go menuva datumot/vremeto.
static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}

// Gi proveruva hostovite idx[0..k-1] istovremeno i go polni results[idx[i]].
static void run_round(char **hosts, const int *idx, int k, int T, struct Result *results)
{
    pid_t pid[k];
    double start[k];
    int running[k];
    int left = k;

    for (int i = 0; i < k; i++) {
        // Go prazni baferot pred fork: inaku deteto go nasleduva nepraznoto
        // kopie na baferot i prompt-ot bi se pecatel dvapati.
        fflush(stdout);
        pid[i] = fork();
        if (pid[i] < 0) {
            perror("fork");
            running[i] = 0;
            results[idx[i]].status = "DOWN";
            results[idx[i]].secs = 0;
            left--;
            continue;
        }
        if (pid[i] == 0) {
            // Dete: go isklucuva izlezot na ping. I stderr, bidejki ping
            // pecati greski (na pr. nepoznato ime) koi ne se del od rezultatot.
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
            execlp("ping", "ping", "-c", "1", hosts[idx[i]], (char *)NULL);
            // Ako exec ne uspee: _exit, ne exit. exit() bi gi isprazhnil
            // baferite na stdio kopirani od roditelot i bi pecatel dupliran izlez.
            _exit(127);
        }
        start[i] = now_s();
        running[i] = 1;
    }

    while (left > 0) {
        for (int i = 0; i < k; i++) {
            if (!running[i])
                continue;
            struct Result *res = &results[idx[i]];
            int st;
            pid_t r = waitpid(pid[i], &st, WNOHANG);
            if (r == pid[i]) {
                res->secs = (int)(now_s() - start[i]);   // celi sekundi, kako vo primerot
                res->status = (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? "UP" : "DOWN";
                running[i] = 0;
                left--;
            } else if (r == 0 && now_s() - start[i] >= T) {
                kill(pid[i], SIGKILL);
                waitpid(pid[i], &st, 0);   // zhetva: bez ova deteto ostanuva zombie
                res->status = "TIMEOUT";
                res->secs = T;
                running[i] = 0;
                left--;
            } else if (r == -1) {
                perror("waitpid");
                res->status = "DOWN";
                res->secs = 0;
                running[i] = 0;
                left--;
            }
        }
        if (left > 0) {
            // 100 ms a ne sleep(1): brzite hostovi treba da pokazat 0 s;
            // so sleep(1) prvata proverka bi bila duri po 1 s i bi pokazalo 1 s.
            struct timespec ts = {0, 100000000L};
            nanosleep(&ts, NULL);
        }
    }
}

int main(int argc, char *argv[])
{
    int M = 1;
    int first = 1;

    if (argc >= 2 && strcmp(argv[1], "-r") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Upotreba: %s [-r M] host...\n", argv[0]);
            return 1;
        }
        char *end;
        long m = strtol(argv[2], &end, 10);
        if (*argv[2] == '\0' || *end != '\0' || m < 1 || m > 1000000) {
            fprintf(stderr, "Greshka: M mora da bide cel broj >= 1\n");
            return 1;
        }
        M = (int)m;
        first = 3;
    }
    if (argc - first < 1) {
        fprintf(stderr, "Upotreba: %s [-r M] host...\n", argv[0]);
        return 1;
    }

    char **hosts = argv + first;
    int n = argc - first;

    int T;
    printf("Vnesi timeout vo sekundi: ");
    fflush(stdout);
    if (scanf("%d", &T) != 1 || T < 1) {
        fprintf(stderr, "Greshka: timeout mora da bide cel broj >= 1\n");
        return 1;
    }

    struct Result results[n];
    int idx[n];
    for (int i = 0; i < n; i++) {
        results[i].host = hosts[i];
        results[i].status = "DOWN";
        results[i].secs = 0;
        results[i].attempts = 1;
        idx[i] = i;
    }
    run_round(hosts, idx, n, T, results);

    // M = vkupen broj obidi (prviot se broi): -r 3 => najmnogu 3 proverki
    // vkupno, "posle 3 obidi" kako vo primerot.
    for (int attempt = 2; attempt <= M; attempt++) {
        int k = 0;
        for (int i = 0; i < n; i++)
            if (strcmp(results[i].status, "DOWN") == 0)
                idx[k++] = i;
        if (k == 0)
            break;
        sleep(1);
        run_round(hosts, idx, k, T, results);
        for (int j = 0; j < k; j++)
            results[idx[j]].attempts = attempt;
    }

    for (int i = 0; i < n; i++) {
        printf("%s %s (%d s)", results[i].host, results[i].status, results[i].secs);
        if (results[i].attempts > 1)
            printf(" posle %d obidi", results[i].attempts);
        printf("\n");
    }
    printf("Zavrshuva process 0\n");
    return 0;
}
