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
}