// Да се напише програма во C која работи со процеси и нитки. Програмата (главната нитка) како
// аргумент од командна линија добива име на влезна датотека. Во влезната датотека, во секој ред
// одделно, се сместени IP адреси (формат: х.х.х.х, каде х е број од 0 до 255). Главната нитка треба
// да креира онолку нитки колку што има IP адреси во датотеката. Секоја нитка, пресметува колку
// можни IP адреси може да генерира, почнувајќи од IP адресата што ја добива, до 255 бројот за
// секој дел од x.y.z.k. Генерирањето на IP адресите се прави така што к оди до 255, па потоа се
// зголемува z, а к почнува од 0 и т.н. Секоја нитка треба да пресмета колку може да генерира IP
// адреси. На крај, главниот процес(нитка) печати на екран, колку вкупно (од сите нитки) може да
// се генерираат IP адреси. За конвертирање на низа од знаци во цел број, може да се користи
// функцијата atoi(char *)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

void *najdi_ip_adresi(void *num){
    int idx = *(int *)num;
    char *ip = ip_adresi[idx];
    int delovi[4];
    if (sscanf(ip, "%d.%d.%d.%d", &delovi[0], &delovi[1], &delovi[2], &delovi[3]) != 4) {
        long long *zero = malloc(sizeof(long long));
        *zero = 0;
        return (void *)zero;
    }
    for (int t = 0; t < 4; t++) {
        if (delovi[t] < 0 || delovi[t] > 255) {
            long long *zero = malloc(sizeof(long long));
            *zero = 0;
            return (void *)zero;
        }
    }

    long long vkupno = 0;
    for(int a = delovi[0]; a < 256; a++){
        for(int b = (a==delovi[0] ? delovi[1] : 0); b < 256; b++){
            for(int c = (a==delovi[0] && b==delovi[1] ? delovi[2] : 0); c < 256; c++){
                for(int d = (a==delovi[0] && b==delovi[1] && c==delovi[2] ? delovi[3] : 0); d < 256; d++){
                    vkupno++;
                }
            }
        }
    }
    long long *rezultat = malloc(sizeof(long long));
    *rezultat = vkupno;
    return (void *)rezultat;
}

char ip_adresi[100][16];

int main(int argc, char *argv[]){
    if(argc <2){
        printf("Nevaliden bron na argumenti.\n");
        return 1;
    }

    FILE *fp = fopen(argv[1],"r");
    if (fp == NULL){
        printf("Ne mozam da ja otvoram datotekata %s\n",argv[1]);
        return 1;
    }

    int broj_ip = 0;

    while(broj_ip < 100 && fscanf(fp, "%15s", ip_adresi[broj_ip]) == 1){
        broj_ip++;
    }

    pthread_t nitki[broj_ip];
    int n_br[broj_ip];
    long long vkupno_ip = 0;

    for (int i=0;i<broj_ip;i++){
        n_br[i]=i;
        pthread_create(&nitki[i],NULL,najdi_ip_adresi,(void *)(n_br+i));
    }

    void *rezultat;
    for (int i=0;i<broj_ip;i++){
        pthread_join(nitki[i], &rezultat);
        vkupno_ip += *(long long *)rezultat;
        free(rezultat);
    }
    printf("Vkupno mogni IP adresi: %lld\n", vkupno_ip);
        fclose(fp);

    return 0;
}

