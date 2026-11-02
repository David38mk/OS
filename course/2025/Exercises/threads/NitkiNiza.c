#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

int pole[1000];
int maxN[10];

void * generiraj(void * brn){
int i = *((int*)brn);
srand(time(0)+i);
for(int j=i*100;j<i*100+100;j++){
  pole[j] = rand();
}
pthread_exit(0);
}

void * maksimumi(void *brn){
  int i = *((int*)brn);
  maxN[i] = pole[i*100];
  for(int j=i*100;j<i*100+100;j++){
    if(maxN[i]<pole[j]){
      maxN[i] = pole[j];
    }
  }
  pthread_exit(0);
}

int main(int argc, char*argv[]){
  pthread_t nitka[10];
  int brn[10];
  int max;
  for(int i =0;i<10;i++){
    brn[i]=i;
    pthread_create(&nitka[i],NULL,generiraj,(void*)&brn[i]);
  }
  for(int i =0;i<10;i++){
    pthread_join(nitka[i],NULL);
  }
  for(int i =0;i<10;i++){
    brn[i]=i;
    pthread_create(&nitka[i],NULL,maksimumi,(void*)&brn[i]);
  }
  for(int i =0;i<10;i++){
    pthread_join(nitka[i],NULL);
  }
  max = maxN[0];
  for(int i=0;i<10;i++){
    if(max<maxN[i]){
      max = maxN[i];
    }
  }
  printf("Najgolemiot broj e %d",max);
  return 0;
}
