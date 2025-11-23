//  Да се напише програма во C програмскиот јазик за работа со процеси и нитки. Програмата како аргументи од командна
// линија добива имиња на датотеки во кои има вредности за различни крипто-валути. Во секоја посебна датотека има реални
// вредности одделени со празно место од посебни крипто-валути. Името на датотеката е името на крипто-валутата. Главниот процес
// треба да креира онолку низи со реални вредности колку што има датотеки (крипто-валути) наведни како аргументи од командна
// линија и да ги смести во секоја низа одделните вредности прочитани од датотеката (најмногу 100 датотеки – криптовалути може да
// се наведат). Потоа, главниот процес креира дете процес на кој му ги испраќа овие низи од крипто-валути и го чека дете процесот да
// заврши.
// Дете процесот најпрво треба да креира онолку нитки колку што има низи (крипто-валути), притоа секоја нитка пребарува посебна
// низа (крипто-валута). Нитките низ низите треба да го најдат најголемото зголемување помеѓу две соседни вредности и враќа кое е
// најголемото зголемување за соодветната крипто-валута. Откако ќе завршат сите нитки, главната нитка (дете-процес) за секоја
// крипто-валута го печати името на крипто-валутата и колкаво е најголемото зголемување за истата.
// БОНУС: главната нитка, завршното печатење да не го прави со помош на printf функцијата туку со помош на echo Shell командата

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

void  *najgolemo_zgolemuvanje(void *num){
    float *res = malloc(sizeof(float));
    int broj_nitka = *(int*)num;
    if (!res) pthread_exit(NULL);
    float max_zgolemuvawe = 0.0;
    for (int i=1; i<nizi_size[broj_nitka]; i++){
        float zgolemuvawe = nizi[broj_nitka][i] - nizi[broj_nitka][i-1];
        if (zgolemuvawe > max_zgolemuvawe){
            max_zgolemuvawe = zgolemuvawe;
        }
    }
    *res = max_zgolemuvawe;
    return (void*)res;
}

int nizi_size[100];
float nizi[100][100];

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Nevaliden broj na argumenti.\n");
        return 1;
    }
    int br_datoteki = argc - 1;
    

    for (int i=0; i< br_datoteki; i++){
        FILE *fp = fopen(argv[i+1], "r");
        if (fp == NULL){
            printf("Ne mozam da ja otvoram datotekata %s\n", argv[i+1]);
            return 1;
        }
        int j = 0;
        while (j < 100 && fscanf(fp, "%f", &nizi[i][j]) == 1){
            j++;
        }
        nizi_size[i] = j;
        fclose(fp);
    }

    pid_t dete;

    dete = fork();

    if(dete<0){
        printf("Fork failed \n");
        return 1;
    }else if(dete==0){
        pthread_t nitki[br_datoteki];
        int nitki_broj[br_datoteki];
        for (int i=0; i<br_datoteki; i++){
            // kreiranje na nitki
            nitki_broj[i] = i;
            pthread_create(&nitki[i], NULL, najgolemo_zgolemuvanje, (void *)nitki_broj[i]);
        }
        float rezultati[br_datoteki];
        for (int i = 0; i < br_datoteki; i++) {
            void *ret;
            pthread_join(nitki[i], &ret);
            if (ret) {
                rezultati[i] = *(float *)ret;
                free(ret);
            } else {
                rezultati[i] = 0.0f;
            }
        }
        // pecatenje na rezultatite
        for (int i=0; i<br_datoteki; i++){
            printf("Najgolemo zgolemuvanje za %s e: %.2f\n", argv[i+1], rezultati[i]);
        }

    }else{
        wait(NULL);
    }
    return 0;
}
