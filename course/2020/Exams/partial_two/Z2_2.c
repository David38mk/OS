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
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int id = atoi(argv[1]);
    
    key_t kluc = 1234;
    int memID = shmget(kluc, 1024, 0666 | IPC_CREAT);

    if(memID == -1){
        printf("Memorijata ne e kreirana");
        return 1;
    }

    char *dmem = (char*)shmat(memID, NULL, 0);

    if(dmem == (char*)-1){
        printf("Pokazuvacot ne uspea da ja pristapi memorijata")
        return 1;
    }

    const char *ime_dat = "dat.txt";

    int fd = open(ime_dat, O_RDWR | O_CREAT, 0666);

    if(fd == -1){
        printf("NEUSPESNO OTVORANJE NA DATOTEKA");
        return 1;
    }
    ftruncate(fd, 1024);

    char *mm = (char*)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(mm == MAP_FAILED){
        printf("ERROR");
        return 1;
    }

    

    if(id == 0){
        dmem[0] = '0';
        char naredba[30];
        while(1){
            if(dmem[0]!='0'){
                sleep(1);
            }else{
                scanf("%s",naredba);
                write();
            }
        }
    }else{

    }

    return 0;
}