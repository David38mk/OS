#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <phtread.h>
#include <unistd.h>

int main(int argc, char* argv[]){
  int K,i,brojac=0;
  scanf("%d",&K);
  char komanda[30];
  char arg1[30], arg2[30], arg3[30];
  pid_t dete[K];
  char komandi[K][4][100];
  if(K>100 || K<0){
    printf("Nevalidna vrednost na K");
    return 0;
  }
  for(i=0;i<K;i++){
    scanf("%99s %99s %99s %99s",komandi[i][0],komandi[i][1],komandi[i][2],komandi[i][3]);
  }
  srand(time(NULL));
  for(i=0;i<K;i++){
    dete[i] = fork();
    if(dete[i] == 0){
      int slBr = rand()%20+1;
      int count = 0;
      while(count<slBr){
        sleep(1);
        count++;
      }
      brojac+=count;
      printf("Jas dete br %d cekam %d sekundi pred da pocnam",i,count);
              execlp(komandi[i][0],
               komandi[i][0],
               komandi[i][1][0] ? komandi[i][1] : NULL,
               komandi[i][2][0] ? komandi[i][2] : NULL,
               komandi[i][3][0] ? komandi[i][3] : NULL,
               (char *)NULL);
    }
  }
  if(i>K){
    for(i=0;i<K;i++){
      wait(NULL);
    }
    printf("SIte zavrsija so vkupno vreme na cekanje %d", brojac);
  }
  return 0;
}
