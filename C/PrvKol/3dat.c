#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <singal.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struck{
  char *dat;
  char *word;
  int i;
}info;

pthread_t nitki[100];
info data[100];
int countWord[100];
int vkupno=0;

void * countword(void * data){
info pod = *((info*)data);
FILE *fp = fopen(pod->dat,"r");
if(fp==NULL){
  printf("error ne uspea da ja otvori datotekata");
  pthread_exit(NULL);
}
countWord[pod.i]=0;
char c[100];
while(fscanf(fp,"%s",c)!=EOF){
  if(strcmp(c,pod->word)==0){
      countWord[i];
      vkupno++;
  }
}
fclose(fp);
printf("Datotekata %s go sodrzi zborot %s tocno %d pati",pod->dat,pod->word,countWord[i]);
pthread_exit(NULL);

}

int main(int argc, char* argv[]){
  if(argc<2){
    char *zb;
    char ar[3][20];
    scanf("%s",&zb);
    scanf("%s",&ar[0]);
    scanf("%s",&ar[1]);
    scanf("%s",&ar[2]);
    for(int i=0;i<argc-2;i++){
      data.i=i;
      data->word = *zb;
      data->dat = ar[i];
      pthread_create(&nitki[i],NULL,countword,(void*)&data[i]);
    }
    for(int i=0;i<argc-2;i++){
      pthread_join(nitki[i],NULL);
    }
    for(int i=0;i<argc-2;i++){
      printf("Vo datotekata %s brojot se pojavis so %d %%",data[i].dat,countWord[i]/vkupno);
    }
  }else if(argc<3){
    char ar[3][20];
    scanf("%s",&ar[0]);
    scanf("%s",&ar[1]);
    scanf("%s",&ar[2]);
    for(int i=0;i<argc-2;i++){
      data.i=i;
      data->word = argv[1];
      data->dat = ar[i];
      pthread_create(&nitki[i],NULL,countword,(void*)&data[i]);
    }
    for(int i=0;i<argc-2;i++){
      pthread_join(nitki[i],NULL);
    }
    for(int i=0;i<argc-2;i++){
      printf("Vo datotekata %s brojot se pojavis so %d %%",data[i].dat,countWord[i]/vkupno);
    }
  }else{
    for(int i=0;i<argc-2;i++){
      data.i=i;
      data->word = argv[1];
      data->dat = argv[i];
      pthread_create(&nitki[i],NULL,countword,(void*)&data[i]);
    }
    for(int i=0;i<argc-2;i++){
      pthread_join(nitki[i],NULL);
    }
    for(int i=0;i<argc-2;i++){
      printf("Vo datotekata %s brojot se pojavis so %d %%",data[i].dat,countWord[i]/vkupno);
    }
  }
  
  return 0;
}
