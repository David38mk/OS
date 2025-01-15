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
    int pipe1[2], pipe2[2];
    pid_t dete1, dete2;
    pipe(pipe1);
    dete1 = fork();
    if(dete1>0){
        close(pipe1[0]);
        dup2(pipe1[1],1);
        execlp("cat", "cat", "dat.txt", NULL);
    }else{
        pipe(pipe2);
        dete2 = fork();
        if(dete2>0){
            close(pipe1[1]);
            close(pipe2[0]);
            int z;
            scanf("%d",&z);
            if(z==0){
                dup2(pipe1[0],0);
                dup2(pipe2[1],1);
                execl("tail", "tail", 9, NULL);
            }else{
                char buffer[1024];
                ssize_t procitano;
                while ((procitano = read(pipe1[0],buffer,1024))>0)
                {
                    buffer[procitano] = '\0'
                    printf("%s",buffer);
                }
                dup2(pipe2[1],1);
                execl("tail", "tail", "9", NULL);
            }
        }else{
            close(pipe2[1]);
            int z;
            scanf("%d",&z);
            if(z==0){
                dup2(pipe2[0],0);
                execl("wc", "wc", "-c", NULL);
            }else{
                char buffer[1024];
                ssize_t procitano;
                while ((procitano = read(pipe1[0],buffer,1024))>0)
                {
                    buffer[procitano] = '\0'
                    printf("%s",buffer);
                }
                execl("wc", "wc", "-c", NULL);
            }
        }
    }
    return 0;
}