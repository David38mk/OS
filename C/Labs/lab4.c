#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define mem_size 1024

int main(int argc, char *argv[]){
    if(argc!=3){
        printf("Nevaliden broj na argumenti");
        return 1;
    }

    int tip = atoi(argv[1]);
    key_t kluc = atoi(argv[2]);
    int memID = shmget(kluc, mem_size, 0666 | IPC_CREAT);

    if(memID){
        printf("Memorijata ne e kreirana");
        return 1;
    }

    char *dmem = (char*)shmat(memID, NULL, 0);

    if(dmem == (char*)-1){
        printf("Pokazuvacot ne uspea da ja pristapi memorijata")
        return 1;
    }

    srand(time[NULL]);

    if(tip == 1){
        while(1){
            printf("Vnesi poraka: ");
            fgets(dmem +1, mem_size -1, stdin);
            dmem[0] = 1;
            int spienje = rand() % 10+1;
            sleep(spienje);
        }
    }else if(tip == 2){
        while(1){
            sleep(5);
            memset(dmem,0,mem_size);
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
    shmdt(dmem);
    return 0;
}