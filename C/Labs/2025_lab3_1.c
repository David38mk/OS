// Да се напише C програма која ќе креира три типови (типот се праќа како прв аргумент од
// командна линија) на процеси кои ќе комуницираат помеѓу себе преку заедничка shared“
// меморија. Како втор аргумент се праќа клучот преку кој ќе комуницираат процесите од
// заедничката меморија.
// Првиот тип на процес чита од тастатура одредена порака и ја запишува во меморијата и
// сигнализира на третиот тип на процес да прочита од меморијата и заспива одреден број на
// секунди (случајно се одбира од 1 до 10 со функцијата rand()%10+1). Откако ќе се разбуди, доколку
// е избришана пораката почнува од почеток со читање на порака од тастатура и така натаму.
// Доколку не е избришана пораката, на секоја една секунда проверува дали е избришана.
// Третиот тип на процес доколку има што да прочита од меморијата, го чита и печати на екран и
// заспива исто како и првиот тип на процес (случајно се одбира од 1 до 10 со функцијата
// rand()%10+1). Потоа, на секоја една секунда проверува дали има нешто повторно во меморијата.
// Вториот тип на процес на секои 5 секунди ја брише пораката од заедничката меморија со
// пополнување на нули.
// Помош: може еден бајт од меморијата да се користи за сигнализирање дали има нешто запишано
// во истата. 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define MEM_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Upotreba: %s <tip(1/2/3)> <kluc>\n", argv[0]);
        return 1;
    }

    int type = atoi(argv[1]);
    key_t key = atoi(argv[2]);

    int shmid = shmget(key, MEM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    char *shared = (char *) shmat(shmid, NULL, 0);
    if (shared == (char *) -1) {
        perror("shmat");
        return 1;
    }

    srand(time(NULL) ^ getpid());

    printf("Proces tip %d startuvan...\n", type);

    // TYPE 1 - W R I T E R
    if (type == 1) {
        while (1) {
            if (shared[0] == 0) {
                printf("Vnesi poraka: ");
                fflush(stdout);
                fgets(shared + 1, MEM_SIZE - 1, stdin);
                shared[0] = 1;

                int t = rand() % 10 + 1;
                printf("Spijam %d sekundi...\n", t);
                sleep(t);
            } else {
                sleep(1);
            }
        }
    }

    // TYPE 2 - D E L E T E R
    else if (type == 2) {
        while (1) {
            sleep(5);
            memset(shared, 0, MEM_SIZE);
            printf("Porakata e izbrisana.\n");
        }
    }

    // TYPE 3 - R E A D E R
    else if (type == 3) {
        while (1) {
            if (shared[0] == 1) {
                printf("Procitano: %s", shared + 1);

                int t = rand() % 10 + 1;
                printf("Procital, spijam %d sekundi...\n", t);
                sleep(t);
            } else {
                sleep(1);
            }
        }
    }

    else {
        printf("Nevaliden tip! Koristi 1, 2 ili 3.\n");
    }

    shmdt(shared);
    return 0;
}
