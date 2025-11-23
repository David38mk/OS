#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define dat_golemina 1024

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("Nevaliden broj na argumenti");
        return 1;
    }

    int tip = atoi(argv[1]);
    const char *ime_dat = argv[2];

    int fd = open(ime_dat, O_RDWR | O_CREAT, 0666);

    if(fd == -1){
        printf("NEUSPESNO OTVORANJE NA DATOTEKA");
        return 1;
    }
    ftruncate(fd, dat_golemina);

    char *dmem = (char*)mmap(NULL, dat_golemina, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(dmem == MAP_FAILED){
        printf("ERROR");
        return 1;
    }

    srand(time(NULL));

    if(tip == 1){
        while(1){
            printf("Vnesi poraka: ");
            fgets(dmem +1, dat_golemina -1, stdin);
            dmem[0] = 1;
            int spienje = rand() % 10+1;
            sleep(spienje);
        }
    }else if(tip == 2){
        while(1){
            sleep(5);
            memset(dmem,0,dat_golemina);
        }
    }else if(tip == 3){
        while(1){
        if(dmem[0] == 1){
            printf("Citam %s", dmem+1);
            dmem[0]=0;
            int spienje= rand()%10+1;
            sleep(spienje);
        }else{
            sleep(1);
        }
        }
    }else{
        printf("Nevaliden tip");
    }

    munmap(dmem, dat_golemina);
    close(fd);

    return 0;
}
