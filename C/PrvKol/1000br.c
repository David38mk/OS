// Да се напише програма во C која работи со процеси и нитки. Главната програма (родител процесот)
// пополнува низа од 1000 броеви на случаен начин со помош на rand() функцијата. Потоа, креира два
// деца процеси, така што, двата деца процеси вршат пребарување на бројот 0 во низата од 1000 броеви.
// Првото дете процес, пребарувањето го прави со помош на 10 деца процеси, додека пак второто дете,
// пребарувањето го прави со 10 нитки. Секоја нитка/дете процес добива дел од низата што треба да го
// пребара бројот 0 и печати на екран колку пати е пронајден бројот кај соодветната нитка/дете процес.
// Родител процесот чека да завршат двете деца процеси и на екран печати кое од двете завршило прво.
// Доколку прво заврши дете процесот кое пребарувањето го прави со помош на деца процеси, тогаш на
// екран се печати „pobrzo zavrshi prebaruvanjeto so deca procesi“, инаку печати „pobrzo zavrsi prebaruvanjetopobrzo zavrshi prebaruvanjeto so deca procesi“, инаку печати „pobrzo zavrshi prebaruvanjeto so deca procesi“, инаку печати „pobrzo zavrsi prebaruvanjetopobrzo zavrsi prebaruvanjeto
// so nitki“.

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

int niza[1000];


void * prebaraj(void*indexi){
int i = *((int*)indexi);
int found = 0;
for(int j=i*100;j<i*100+100;j++){
  if(niza[j]==0){
    found++;
  }
}
printf("Nikta %d najde %d br 0\n",i,found);
}

//bonus
// void * prebaraj(){
//   int i = *((int*)indexi);
//   int found = 0;
//   for(int j=i*100;j<i*100+100;j++){
//     if(niza[j]==0){
//       found++;
//     }
//   }
//   najdeni_0_vo_nitki += found;
//   pthread_exit(NULL);
// }

// int najdeni_0_vo_nitki=0;

int main(){
  srand(time(0));
  int i, found=0, j;
  pthread_t nitki[10];
  int indexi[10];
  for(i=0;i<1000;i++){
    niza[i] = rand()%10;
  }
  pid_t dete1, dete2;
  dete1 = fork();
  pid_t dete[10];
  if(dete1==0){
    pid_t dete[10];
    for(i=0;i<10;i++){
      dete[i] = fork();
      if(dete[i]==0){
        break;
      }
    }
    if(i<10){
    found =0;
      for(j=i*100;j<i*100+100;j++){
        if(niza[j]==0){
          found ++;
        }
      }
      // printf("Deteto %d najde %d br 0\n",i, found);
      // FILE *fp = fopen("communicate.txt","w");
      // fprintf(fp,"%d\n",found);
      // fclose(fp);
    }else{
    for(i=0;i<10;i++){
      wait(NULL);
    }
    //bonus
    // FILE *fp = fopen("communicate.txt","r");
    // int total_found =0;
    // for(i=0;i<10;i++){
    //   int temp;
    //   fscanf(fp,"%d",&temp);
    //   total_found += temp;
    // }
    // fclose(fp);
    // printf("Deteto so procesi najde vkupno %d br 0
  }
  }else{
    dete2 = fork();
    if(dete2==0){
      for(i=0;i<10;i++){
        indexi[i]=i;
        pthread_create(&nitki[i],NULL,prebaraj,(void*)&indexi[i]);
      }
      for(i=0;i<10;i++){
        pthread_join(nitki[i],NULL);
      }
      // bonuse
      // printf("Najdeni se vkupno %s \n",najdeni_0_vo_nitki);
    }else{
        if(wait(NULL)==dete1){
          printf("Zavrsi deteto so procesi");
        }else{
          printf("Zavrsi deteto so nitki");
        }
    }
  }
  
  return 0;
}
