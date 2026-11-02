#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

void * pechati(void * redenBrNitki){
  int i = *((int*)redenBrNitki);
  printf("Brojot na ovaa nitka e %d\n",i);
  pthread_exit(0);
}

int main(int argc, char *argv[]){
  pthread_t nitki[5];
  int redenBrNitki[5];
  int i;
  for(i=0;i<5;i++){
    redenBrNitki[i]=i;
    int rc = pthread_create(&nitki[i],NULL,pechati,(void*)&redenBrNitki[i]);
    if(rc){
      printf("Greska");
      return -1;
    }
  }
  for(i=0;i<5;i++){
    printf("Ja cekam nitkata so reden broj %d\n",redenBrNitki[i]);
    pthread_join(nitki[i],NULL);
  }
  return 0;
  
}
