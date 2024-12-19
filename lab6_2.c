#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
// #include <sys/wait.h>

#define Buffer 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Nevaliden broj na argumenti");
        return -1;
    }

    int pipe1[2], pipe2[2];

    if(pipe(pipe1)){
        fprintf(stderr,"pipe failed\n");
        return EXIT_FAILURE;
    }

    if(pipe(pipe2)){
        fprintf(stderr,"pipe failed\n");
        return EXIT_FAILURE;
    }
    pid_t pid1, pid2, pid3;

    if ((pid1 = fork()) == 0) {
        close(pipe1[0]); 
        close(pipe2[0]); 
        close(pipe2[1]);

        char command[Buffer];
        printf("Vnesete komanda za proces 1: ");
        fgets(command, Buffer, stdin);
        command[strcspn(command, "\n")] = 0; 

        char *args[argc + 2];
        args[0] = command;
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;

        dup2(pipe1[1], STDOUT_FILENO); 
        close(pipe1[1]);

        execvp(command, args);
        pintf("Neuspesno isvrsena comanda execvp");
        exit(EXIT_FAILURE);
    } else if ((pid2 = fork()) == 0) {
        close(pipe1[1]); 
        close(pipe2[0]); 

        char command[Buffer];
        printf("Enter command for process 2: ");
        fgets(command, Buffer, stdin);
        command[strcspn(command, "\n")] = 0; 

        char *args[argc + 2];
        args[0] = command;
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;

        dup2(pipe1[0], STDIN_FILENO);  
        dup2(pipe2[1], STDOUT_FILENO); 
        close(pipe1[0]);
        close(pipe2[1]);

        execvp(command, args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if ((pid3 = fork()) == 0) {
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]); 

        char command[Buffer];
        printf("Enter command for process 3: ");
        fgets(command, Buffer, stdin);
        command[strcspn(command, "\n")] = 0; 

        char *args[argc + 2];
        args[0] = command;
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;

        dup2(pipe2[0], STDIN_FILENO); 
        close(pipe2[0]);

        execvp(command, args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
       
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        waitpid(pid3, NULL, 0);
    }

    return 0;
}
