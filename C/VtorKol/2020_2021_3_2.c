#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>



int main(int argc, char *argv[]){
    pid_t dete;
    int pipe1[2];
    pipe(pipe1);

    dete = fork();

    if(dete>0){
        close(pipe1[0]);

        dup2(pipe1[1],1);
        char tekst[100];
        scanf("%s",tekst);
        printf("%s",tekst);
    }else{
        close(pipe1[1]);
        char mem[100];
        char vnes[100];
        int fd = open("dat.txt",O_RDONLY,0666);
        dup2(pipe1[0],0);
        scanf("%s",mem);

        dup2(fd, 0);
        scanf("%s",vnes);

        if(!strcmp(mem,vnes)){
            printf("Isti se");
        }else{
            printf("Ne se");
        }
    }    
}