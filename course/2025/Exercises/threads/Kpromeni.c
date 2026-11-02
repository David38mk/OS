#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

int niza[100];
int promeni=0;
int N;
int k;

void * zgolemi(void * k){
  int i = *((int*)k);
  for(int j=0;j<i;j++){
    niza[rand()%100]+=1;
    promeni++;
  }
}

void * namali(void * k){
  int i = *((int*)k);
  for(int j=0;j<i;j++){
    niza[rand()%100]-=1;
    promeni++;
  }
}

int main(){
  scanf("%d",&N);
  pid_t dete;
  pthread_t nitki[N];
  int indexi[N];
  dete = fork();
  if(dete == 0){
    for(int i=0;i<N;i++){
      if(i<N/2){
        k = rand()%500+1;
        pthread_create(&nitki[i],NULL,zgolemi,(void*)&k);  
      }else{
        k = rand()%500+1;
        pthread_create(&nitki[i],NULL,namali,(void*)&k);  
      }
    }
    for(int i=0;i<N;i++){
      pthread_join(nitki[i],NULL);
    }
    printf("Napraveni se %d promeni",promeni);
  }else{
    wait(NULL);
  }
  return 0;
}
