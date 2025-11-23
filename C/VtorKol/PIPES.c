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
    }
}