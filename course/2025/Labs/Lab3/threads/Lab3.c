#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

//programa 1

int N,K;
int *niza;
int PIDvtora;

void *prebaraj(void *indN){
  int ind = *((int*)indN);
  int guess;
  for(int i=0;i<N;i++){
    guess = rand()%N;
    if(niza[guess]==K){
      kill(PIDvtora, SIGKILL);
      printf("Nitka so broj %d na proces 1 go najde brojot prva\n",ind);
      exit(0);
    }
  }
  pthread_exit(0);
}

int main(int argc, char *argv[]){
  if(argc!=3){
    printf("Vnesovte nevaliden broj na argumenti!");
    return 0;
  }
  FILE *fp = fopen("dat.txt","r");
  if(fp==NULL){
    printf("programata 1 ne uspea da ja otvori datotekata");
    return 0;
  }
  N = atoi(argv[1]);
  K = atoi(argv[2]);
  niza = malloc(N*sizeof(int));
  if (niza == NULL) {
        printf("Memory allocation failed!\n");
        fclose(fp);
        return 0;
  }
  for(int i = 0; i<N;i++){
    fscanf(fp,"%d",&niza[i]);
  }
  fclose(fp);
  
  PIDvtora = fork();
  if(PIDvtora==0){
    execl("./lab31","lab31",argv[1],argv[2],NULL);
  }
  srand(time(NULL));
  pthread_t nitki[5];
  int indNitki[5];
  for(int i = 0;i<5;i++){
    indNitki[i] = i;
    pthread_create(&nitki[i],NULL,prebaraj,(void*)&indNitki[i]);
  }
  for(int i = 0;i<5;i++){
    pthread_join(nitki[i],NULL);
  }
  kill(PIDvtora, SIGKILL);
  printf("Programa 1 zavrsi prva bez da go najde brojot");
  return 0;
}

