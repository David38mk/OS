#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define mem_size 1024

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("greska");
        return -1;
    }
    int i,Z;
    key_t kluc = 1234;
    int memID = shmget(kluc, mem_size, 0666 | IPC_CREAT);
    time_t startingTime, endingTime;

    if(memID == -1){
        printf("Memorijata ne e kreirana");
        return -1;
    }

    char *dmem = (char*)shmat(memID, NULL, 0);

    if(dmem == (char*)-1){
        printf("Pokazuvacot ne uspea da ja pristapi memorijata")
        return -1;
    }

    scanf("%d", &Z);
    pid_t dete[Z];
    time(&startingTime);
    dmem[0] = 0;
    for(i=0;i<Z;i++){
        dete[i] = fork();
        if(dete[i] == 0){
            break;
        }
    }
    if(i==Z){
        for(int j=0;j<Z;j++){
            //wait_pid(dete[j],0,WNOHANG);
            wait(NULL);
        }
        time(&endingTime);
        printf(difftime(endingTime,startingTime));
    }else{
        while(dmem[0] != i){
            sleep(1);
        }
        dmem[0] = i+1;
        execvp(argv[1],&argv[1]);
    }
    shmdt(dmem);
    shmctl(memID, IPC_RMID, NULL);
    return 0;
}