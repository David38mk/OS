#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#define SIZE 512
int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Greska");
        return 1;
    }
    if(atoi(argv[2])==1){
        int fd;
        mode_t rezim = 0666;
        if((fd=open("pipe1.txt",O_RDWR | O_CREAT,rezim))==-1){
            printf("Greska pri otvoranje na datotekata!");
            return 1;
        }
        //dup2(ekran,1);
        dup2(fd,1);
        char s[100];
        while (1) {
        scanf("%s", s);
        if (strcmp(s, "\0") == 0) {
            break;
        }
         printf("%s ", s); 
        }
        //vtora zadaca:
        //ssize_t procitano;
        //int ekran=dup(1);
        //lseek(fd, 0, SEEK_SET);
        //char memorija[SIZE];
        //while((procitano=read(fd,memorija,SIZE))>0){
        //     for(int i=0;i<SIZE;i++){
        //         printf("%c",memorija[i]);
        //     }
        // }
        close(fd);
    }else if(atoi(argv[2])==2){
        int fd1, fd2;
        mode_t rezim = 0666;
        char memorija[SIZE];
        ssize_t procitano;
        if((fd1=open("pipe1.txt", O_RDWR | O_CREAT,rezim))==-1){
            printf("Greska pri otvoranje na datotekata!");
            return 1;
        }
        if((fd2=open("pipe2.txt", O_RDWR | O_CREAT,rezim))==-1){
            printf("Greska pri otvoranje na datotekata!");
            return 1;
        }
        while((procitano=read(fd1,memorija,SIZE))==SIZE){
            write(fd2,memorija,SIZE);
            //vtora zadaca:
            // for(int i=0;i<SIZE;i++){
            //     printf("%c",memorija[i]);
            // }
        }
        write(fd2,memorija,procitano);
        // for(int i=0;i<procitano;i++){
        //     printf("%c",memorija[i]);
        // }
        close(fd1);
        close(fd2);
    }else if(atoi(argv[2])==3){
        int fd;
        mode_t rezim = 0666;
        char memorija[SIZE];
        ssize_t procitano;
        if((fd=open("pipe2.txt",O_RDWR | O_CREAT,rezim))==-1){
            printf("Greska pri otvoranje na datotekata!");
            return 1;
        }
        int counter = 0;
        while ((procitano=read(fd,memorija,SIZE))==SIZE)
        {
            for(int i=0;i<SIZE;i++){
                if(memorija[i]==' '){
                    counter++;
                }
            }
        }
        for(int i=0;i<procitano;i++){
            if(memorija[i]==' '){
                counter++;
            }
        }
        printf("Procesot broj %d izbroi %d",atoi(argv[1]),counter);
        close(fd);
    }else{
        printf("Nevaliden argument!");
        return 1;
    }
}