#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

void * matfunk(void* br){
  int i = *((int*)br);
  if(i==0){
    printf("2+2=%d\n",2+2);
  }else if(i==1){
    printf("2*2=%d\n",2*2);
  }else if(i==2){
    printf("2-2=%d\n",2-2);
  }else{
    printf("2/2=%d\n",2/2);
  }
  pthread_exit(NULL);
}

int main(int argc, char * argv[]){
  pthread_t nitka[4];
  int brn[4];
  for(int i =0 ;i<4;i++){
    brn[i] =i;
   int rc= pthread_create(&nitka[i],NULL,matfunk,(void*)&brn[i]);
    if(rc){
    printf("Nitka failed");
    return -1;
    }
  }
  for(int i =0 ;i<4;i++){
   pthread_join(nitka[i],NULL);
  }
  printf("Glavna nitka kaput");
  return 0;
}
