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

int main(int argc, char * argv[]){
    int X;
    int pipe[10][2];
    pid_t dete[10];
    scanf("%d",&X);
    char *naredbi[X];
    for(int i=0;i<X;i++){
        scanf("%s",naredbi[i]);
    }
    for(int i=0;i<X;i++){
        dete[i] = fork();
        if(dete[i]==0){
            if(i==0){
                pipe(pipe[i]);
                close(pipe[i][0]);
                dup2(piep[i][1],1);
                execlp(naredbi[i],naredbi[i],NULL);
            }else if(i==X-1){
                close(pipe[i-1][1]);
                dup2(pipe[i-1][0],0);
                execlp(naredbi[i],naredbi[i],NULL);
            }else{
                pipe[i];
                close(pipe[i-1][1]);
                close(pipe[i][0]);
                dup2(pipe[i-1][0],0);
                dup2(piep[i][1],1);
                execlp(naredbi[i],naredbi[i],NULL);
            }
        }else{
            break;
        }
    }
    wait(NULL);
    return 0;
}