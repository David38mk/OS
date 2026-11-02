#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define SIZE 512

int main(int argc, char* argv[]) {

    // Upotreba: ./a.out 1  (ili 2 ili 3)
    if (argc < 2) {
        printf("Greska\n");
        return 1;
    }

    int id = atoi(argv[1]);

    if (id == 1) {
        // Komanda1: cita zborovi od tastatura i pishuva vo pipe1.txt I na ekran (Zadaca 2)
        int fd;
        mode_t rezim = 0666;

        if ((fd = open("pipe1.txt", O_WRONLY | O_CREAT | O_TRUNC, rezim)) == -1) {
            printf("Greska pri otvoranje na datotekata!\n");
            return 1;
        }

        int ekran = dup(1);      // zacuvaj originalen stdout (ekran)
        dup2(fd, 1);             // stdout -> pipe1.txt
        close(fd);

        char s[100];
        while (1) {
            scanf("%99s", s);

            // kraj koga korisnik ke vnese 0
            if (strcmp(s, "0") == 0) {
                break;
            }

            // pishuvame vo pipe1.txt (preku stdout=1)
            printf("%s ", s);
            fflush(stdout);

            // Zadaca 2: pishuvame i na ekran
            dprintf(ekran, "%s ", s);
        }

        close(ekran);
    }
    else if (id == 2) {
        // Komanda2: cita od pipe1.txt, kopira vo pipe2.txt I na ekran (Zadaca 2)
        int fd1, fd2;
        mode_t rezim = 0666;
        char memorija[SIZE];
        ssize_t procitano;

        if ((fd1 = open("pipe1.txt", O_RDONLY)) == -1) {
            printf("Greska pri otvoranje pipe1.txt!\n");
            return 1;
        }
        if ((fd2 = open("pipe2.txt", O_WRONLY | O_CREAT | O_TRUNC, rezim)) == -1) {
            printf("Greska pri otvoranje pipe2.txt!\n");
            return 1;
        }

        // sacuvaj ekran pa prenasoci stdout vo pipe2.txt
        int ekran = dup(1);
        dup2(fd2, 1);     // stdout -> pipe2.txt
        close(fd2);

        // kopiranje
        while ((procitano = read(fd1, memorija, SIZE)) > 0) {
            // vo pipe2.txt (preku stdout=1)
            write(1, memorija, procitano);

            // Zadaca 2: i na ekran
            write(ekran, memorija, procitano);
        }

        close(ekran);
        close(fd1);
    }
    else if (id == 3) {
        // Komanda3: cita od pipe2.txt, broi prazni mesta, pecati na ekran
        int fd;
        char memorija[SIZE];
        ssize_t procitano;

        if ((fd = open("pipe2.txt", O_RDONLY)) == -1) {
            printf("Greska pri otvoranje pipe2.txt!\n");
            return 1;
        }

        int counter = 0;

        while ((procitano = read(fd, memorija, SIZE)) > 0) {
            for (ssize_t i = 0; i < procitano; i++) {
                if (memorija[i] == ' ') counter++;
            }
        }

        printf("Procesot broj %d izbroi %d prazni mesta\n", id, counter);
        close(fd);
    }
    else {
        printf("Nevaliden argument!\n");
        return 1;
    }

    return 0;
}
