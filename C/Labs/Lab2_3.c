#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
  if(argc <2){
    printf("Pogresen broj na argumenti");
    return 0;
  }
  int N;
  printf("Vnesi kolku pati kje se izvrsuva naredbata: ");
  scanf("%d",&N);
  for(int i=0;i<N;i++){
    pid_t dete;
    dete = fork();
    if(dete == 0){
      execvp(argv[1],&argv[1]);
      exit(0);
    }
    
  }
  for(int i=0;i<N;i++){
    wait(NULL);
  }
  printf("Zavrsuva procesot");
  return 0;
}
