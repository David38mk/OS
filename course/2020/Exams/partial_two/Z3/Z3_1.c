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

void otkluci(char *kluc, char *tekst, int p){
    int k=strlen(kluc);
    int t=strlen(tekst);
    int i,sum,z=0;
    for(i=0; i<t; i++){
        if(p==0) {
            sum = kluc[z]+tekst[i];
            if(sum>255) {sum-=255;}
        } else {
        sum=kluc[z]-tekst[i];
        if(sum<0){sum+=255;}}
        tekst[i]=sum;i++; z++;if(z==k) z=0;
    }
}


int main(int argc, char *argv[]){
    if(argc!=2){
        printf("Neavliden br na arg");
        return -1;
    }
    pid_t dete;
    
    dete = fork();
    const char *ime_dat = argv[1];

    int fd = open(ime_dat, O_RDWR | O_CREAT, 0666);

    if(fd == -1){
        printf("NEUSPESNO OTVORANJE NA DATOTEKA");
        return 1;
    }
    ftruncate(fd, 1 + 128 + 1024);

    char *dmem = (char*)mmap(NULL, 1 + 128 + 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(dmem == MAP_FAILED){
        printf("ERROR");
        return 1;
    }


    if(dete>0){
        char kluc[128];
        int fd1 = open("key.out", O_RDONLY, 0666);
        read(fd1, kluc, 128);
        while(1){
            while(dmem[0]==0){
                sleep(1);
            }    
            if(strncmp(kluc,dmem+1,128)){
                otkluci(kluc, dmem+128+1, 0);
                dmem[0] = 2;
                sleep(10);
                otkluci(kluc, dmem+128+1, 1);
            }else{
                dmem[0] = '0';
            }    
        }
        
    }else{
        char kluc1[128];
        int i = 0;
        while(i<3){
            scanf("%s",kluc1);
            strcpy(dmem+1,kluc1);
            dmem[0] = '1';
            while(dmem[0]=='1'){
                sleep(1);
            }
            if(dmem[0]=='2'){
                printf("%s",dmem+1+128);
            }else{
                i++;
            }
        }
    }

}