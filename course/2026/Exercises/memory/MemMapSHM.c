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

    if(memID == -1){
        printf("Memorijata ne e kreirana");
        return 1;
    }

    char *dmem = (char*)shmat(memID, NULL, 0);

    if(dmem == (char*)-1){
        printf("Pokazuvacot ne uspea da ja pristapi memorijata")
        return 1;
    }

}