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
// Задача 2: Да се реши задача 1 со помош на мемориско мапирање (наместо Shared memory) каде
// како аргумент ќе се прати името на датотеката што треба да се прикачи во меморискиот простор. 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILE_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Upotreba: %s <tip(1/2/3)> <datoteka>\n", argv[0]);
        return 1;
    }

    int tip = atoi(argv[1]);
    const char *ime_dat = argv[2];

    int fd = open(ime_dat, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, FILE_SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }

    char *mem = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    srand(time(NULL) ^ getpid());

    printf("Proces tip %d startuvan...\n", tip);

    // TYPE 1 - WRITER
    if (tip == 1) {
        while (1) {
            if (mem[0] == 0) {
                printf("Vnesi poraka: ");
                fflush(stdout);
                fgets(mem + 1, FILE_SIZE - 1, stdin);
                mem[0] = 1;

                int t = rand() % 10 + 1;
                printf("Spijam %d sekundi\n", t);
                sleep(t);
            } else {
                sleep(1);
            }
        }
    }

    // TYPE 2 - DELETER
    else if (tip == 2) {
        while (1) {
            sleep(5);
            memset(mem, 0, FILE_SIZE);
            printf("Porakata e izbrisana\n");
        }
    }

    // TYPE 3 - READER
    else if (tip == 3) {
        while (1) {
            if (mem[0] == 1) {
                printf("Procitano: %s", mem + 1);

                int t = rand() % 10 + 1;
                printf("Procital, spijam %d sek\n", t);
                sleep(t);
            } else {
                sleep(1);
            }
        }
    }

    else {
        printf("Nevaliden tip! Mora 1, 2 ili 3.\n");
    }

    munmap(mem, FILE_SIZE);
    close(fd);
    return 0;
}

