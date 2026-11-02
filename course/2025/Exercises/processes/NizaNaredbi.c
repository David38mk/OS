#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

int nizavreme[100];
pid_t nizaProc[100];

void * cekaj(void * indexi){
int i = *((int*)indexi);
int s=0;
while(waitpid(nizaProc[i],NULL,WNOHANG)!=nizaProc[i]){
  sleep(1);
  s++;
}
nizavreme[i] = s;
pthread_exit(NULL);
}

int main(int argc, char*argv[]){
  int N = argc-1;
  pthread_t nitki[100];
  int indexi[100];
  int i;
  for( i=0;i<N;i++){
    nizaProc[i] = fork();
    if(nizaProc[i]==0){
      break;
    }
  }
  if(i<N){
    execlp(argv[i+1],argv[i+1],NULL);
  }else{
    for( i=0;i<N;i++){
    indexi[i]=i;
    pthread_create(&nitki[i],NULL,cekaj,(void*)&indexi[i]);
  }
  for( i=0;i<N;i++){
    pthread_join(nitki[i],NULL);
  }
  for( i=0;i<N;i++){
    printf("Vremeto na proces %s e %d\n",argv[i+1],nizavreme[i]);
  }
  }
  
  return 0;
}

