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
// ПОЕДНОСТАВЕНА ВЕРЗИЈА: сè во main, без помошни функции и структури, времето со time(). Без БОНУС1 (-r).
// Истиот излез како Z2_S.c (целосната верзија со БОНУС1 и монотон часовник).
//
// Зошто `time(NULL) - start[i] > T`, а не `>= T`: time() брои цели секунди. Ако детето е создадено во 10.9 s,
// start е 10; во 12.0 s разликата е веќе 2, а поминале само 1.1 s. Со `>` убиваме најрано по T секунди и
// најдоцна по T+1, никогаш предвреме.
//
// Употреба: ./Z2 host...
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

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

    pid_t pid[n];        // pid na deteto za hostot i
    time_t start[n];     // koga e startuvano
    int secs[n];         // kolku trael
    char *status[n];     // "UP", "DOWN" ili "TIMEOUT"
    int gotovo[n];       // 1 koga rezultatot za hostot i e poznat
    int preostanati = n; // kolku deca ushte rabotat

    fflush(stdout);      // praznime pred fork: inaku sekoe dete go nasleduva baferot
    for (int i = 0; i < n; i++) {
        pid[i] = fork();
        if (pid[i] < 0) {
            perror("fork");
            status[i] = "DOWN"; secs[i] = 0; gotovo[i] = 1; preostanati--;
            continue;
        }
        if (pid[i] == 0) {
            // dete: bez izlez od ping, pa ping -c 1 <host>
            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);
            execlp("ping", "ping", "-c", "1", argv[i + 1], (char *)NULL);
            _exit(127);  // samo ako execlp ne uspee
        }
        start[i] = time(NULL);
        gotovo[i] = 0;
    }

    while (preostanati > 0) {
        for (int i = 0; i < n; i++) {
            if (gotovo[i])
                continue;
            int st;
            pid_t r = waitpid(pid[i], &st, WNOHANG);   // 0 = ushte raboti, pid = zavrshi
            if (r == pid[i]) {
                secs[i] = (int)(time(NULL) - start[i]);
                if (WIFEXITED(st) && WEXITSTATUS(st) == 0)
                    status[i] = "UP";
                else
                    status[i] = "DOWN";
                gotovo[i] = 1; preostanati--;
            } else if (r == 0 && time(NULL) - start[i] > T) {
                kill(pid[i], SIGKILL);
                waitpid(pid[i], &st, 0);   // zhetva, inaku ostanuva zombi
                secs[i] = T;
                status[i] = "TIMEOUT";
                gotovo[i] = 1; preostanati--;
            } else if (r < 0) {
                perror("waitpid");
                status[i] = "DOWN"; secs[i] = 0; gotovo[i] = 1; preostanati--;
            }
        }
        usleep(100000);   // 0.1 s pauza megju proverkite, da ne vrtime vo prazno
    }

    for (int i = 0; i < n; i++)
        printf("%s %s (%d s)\n", argv[i + 1], status[i], secs[i]);
    printf("Zavrshuva process 0\n");
    return 0;
}
