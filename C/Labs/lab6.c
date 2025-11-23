#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define Buffer 1024

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Greska");
        return -1;
    }else{

        int pipe1[2], pipe2[2];
        int fd;

        if(pipe(pipe1)){
            fprintf(stderr,"pipe failed\n");
            return EXIT_FAILURE;
        }

        if(pipe(pipe2)){
            fprintf(stderr,"pipe failed\n");
            return EXIT_FAILURE;
        }

        if(atoi(argv[1])==1){
            char buffer[Buffer];
            close(pipe1[0]);
            close(pipe2[0]);
            close(pipe2[1]);
            while (fgets(buffer, Buffer, stdin)) {
                if (buffer[0] == '\n') {
                    break;
                }
                write(pipe1[1], buffer, strlen(buffer));
            }
            close(pipe1[1]);

        }else if(atoi(argv[1])==2){
            char buffer[Buffer];
            ssize_t procitano;
            close(pipe1[1]);
            close(pipe2[0]);
            while ((procitano = read(pipe1[0],buffer,Buffer))>0)
            {
                write(pipe2[1],buffer,procitano);
            }
            close(pipe1[0]);
            close(pipe2[1]);
        }else if(atoi(argv[1])==3){
            char buffer[Buffer];
            ssize_t procitano;
            int count;
            close(pipe2[1]);
            close(pipe1[0]);
            close(pipe1[1]);
            while ((procitano = read(pipe2[0],buffer,Buffer))>0){
                for(ssize_t i = 0; i< procitano;i++){
                    if(buffer[i]==' '){
                        count++;
                    }
                }
            }
            printf("Prazni mesta se %d\n",count);
            close(pipe2[0]);
        }else{
            printf("Invalid procces number");
            return -1;
        }
    }
    return 0;
}