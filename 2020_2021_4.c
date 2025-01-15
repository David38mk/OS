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

int main(int argc, char *argv[]){
    pid_t dete;
    int pipe1[2], pipe2[2];
    char mem[1024];
    char *p;
    int tastatura, ekran;
    int count;
    pipe(pipe1);
    piep(pipe2);
    dete = fork();
    if(dete>0){
        close(pipe1[0]);
        close(pipe2[1]);

        dup2(0,tastatura);
        
        dup2(pipe1[1],1);
        dup2(pipe2[0],0);

        printf("0");
        read(tastatura,mem,1024);
        printf("1");
        printf("%s", mem);
        char c;
        while(1){
            scanf("%s",c);
            if(c=='0'){
                sleep(1);
            }else{
                printf("0");
                read(tastatura,mem,1024);
                printf("1");
                printf("%s", mem);
            }
        }
    }else{
        close(pipe1[1]);
        close(pipe2[0]);

        dup2(1, ekran);

        dup2(pipe1[0],0);
        dup2(pipe2[1],1);
        char c;
        while(1){
            if(scanf("%s",&c)=='0'){
                sleep(1);
            }else{
                printf("0");
                count = 0;
                scanf("%s",mem);
                p = mem;
                while(*p){
                    if(isAlpha(*p) && isAlpha(*p+1)){
                        count++;
                    }
                    p++;
                }
                write(ekran, itoa(brojac), strlen(itoa(brojac)));
                printf("1");
            }
        }
    }
    return 0;
}