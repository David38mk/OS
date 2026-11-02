#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

//programa 2

int N,K;
int *niza;
int PIDprva;

void *prebaraj(void *indN){
  int ind = *((int*)indN);
  int guess;
  for(int i=0;i<N;i++){
    guess = rand()%N;
    if(niza[guess]==K){
      kill(PIDprva,SIGKILL);
      printf("Nitka so broj %d na proces 2 go najde brojot prva\n",ind);
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
    printf("programata 2 ne uspea da ja otvori datotekata");
    return 0;
  }
  PIDprva = getppid();
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
  kill(PIDprva,SIGKILL);
  printf("Programa 2 zavrsi prva bez da go najde brojot");
  return 0;
}

