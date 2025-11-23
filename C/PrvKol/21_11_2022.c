//  Да се напише програма во C која работи со процеси и нитки. Програмата (главната
// нитка) како аргументи од командна линија добива збор што се пребарува и листа од
// имиња на датотеки. Доколку не добие листа од датотеки од командна линија (добие
// 120
// мин.
// само збор за пребарување), програмата треба да му дозволи да внесе имиња на 3
// датотеки од тастатура. Доколку нема ни збор за пребарување, најпрво го внесува зборот
// од тастатура, а потоа имињата на 3те датотеки. За секоја датотека во која се прави
// пребарување, се креира посебна нитка која го прави пребарувањето, притоа, на секоја
// нитка ѝ се испраќа кој е зборот што треба да го пребарува и името на датотеката во која
// треба да го прави пребарувањето. Секоја нитка го пребарува зборот во својата датотека,
// и штом заврши, на екран го печати името на датотеката и колку пати се појавил зборот.
// Откако ќе завршат сите нитки, главната нитка на екран печати, за секоја датотека
// одделно, колку процентуално се појавил зборот во таа датотека (процент од вкупното
// појавување на зборот во сите датотеки).


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_FILES 100
#define MAX_NAME 256
#define MAX_WORD 256

char global_zbor[MAX_WORD];

void *prebaraj_vo_datoteka(void *arg){
    char *ime_dat = (char *)arg;
    FILE *fp = fopen(ime_dat, "r");
    if (!fp) {
        fprintf(stderr, "Ne mozam da ja otvoram datotekata %s\n", ime_dat);
        int *zero = malloc(sizeof(int));
        *zero = 0;
        return zero;
    }

    char word[MAX_WORD];
    int total = 0, word_occur = 0;
    while (fscanf(fp, "%255s", word) == 1) {
        if (strcmp(word, global_zbor) == 0) {
            word_occur++;
        }
        total++;
    }

    printf("Vo datotekata %s zborot %s se pojavuva %d pati od vkupno %d zborovi.\n",
           ime_dat, global_zbor, word_occur, total);
    fclose(fp);

    int *result = malloc(sizeof(int));
    *result = word_occur;
    return (void *)result;
}

int main(int argc, char *argv[]){
    char zbor_za_prebaruvanje[MAX_WORD];
    char datoteki[MAX_FILES][MAX_NAME];
    char *datoteki_ptrs[MAX_FILES];
    int num_datoteki = 0;

    if (argc < 2) {
        printf("Vnesete zbor za prebaruvanje: ");
        if (scanf("%255s", zbor_za_prebaruvanje) != 1) {
            fprintf(stderr, "Nevaliden vnes.\n");
            return 1;
        }
        num_datoteki = 3;
        for (int i = 0; i < num_datoteki; i++) {
            printf("Vnesi ime na datoteka %d: ", i+1);
            if (scanf("%255s", datoteki[i]) != 1) {
                fprintf(stderr, "Nevaliden vnes za datoteka.\n");
                return 1;
            }
        }
    } else if (argc == 2) {
        strncpy(zbor_za_prebaruvanje, argv[1], MAX_WORD-1);
        zbor_za_prebaruvanje[MAX_WORD-1] = '\0';
        num_datoteki = 3;
        for (int i = 0; i < num_datoteki; i++) {
            printf("Vnesi ime na datoteka %d: ", i+1);
            if (scanf("%255s", datoteki[i]) != 1) {
                fprintf(stderr, "Nevaliden vnes za datoteka.\n");
                return 1;
            }
        }
    } else {
        strncpy(zbor_za_prebaruvanje, argv[1], MAX_WORD-1);
        zbor_za_prebaruvanje[MAX_WORD-1] = '\0';
        num_datoteki = argc - 2;
        if (num_datoteki > MAX_FILES) {
            fprintf(stderr, "Prekumulno datoteki (maks %d). Koristam prvite %d.\n", MAX_FILES, MAX_FILES);
            num_datoteki = MAX_FILES;
        }
        for (int i = 0; i < num_datoteki; i++) {
            strncpy(datoteki[i], argv[i+2], MAX_NAME-1);
            datoteki[i][MAX_NAME-1] = '\0';
        }
    }

    strncpy(global_zbor, zbor_za_prebaruvanje, MAX_WORD-1);
    global_zbor[MAX_WORD-1] = '\0';

    for (int i = 0; i < num_datoteki; i++) {
        datoteki_ptrs[i] = datoteki[i];
    }

    pthread_t nitki[num_datoteki];

    for (int i = 0; i < num_datoteki; i++) {
        if (pthread_create(&nitki[i], NULL, prebaraj_vo_datoteka, datoteki_ptrs[i]) != 0) {
            fprintf(stderr, "Ne mozam da kreiram nitka za %s\n", datoteki_ptrs[i]);
            // create a dummy result on the heap to keep join logic simple
            int *zero = malloc(sizeof(int));
            *zero = 0;
            // we could instead set a flag and skip join, but keep simple
        }
    }

    int *counts = malloc(sizeof(int) * num_datoteki);
    int total_occurrences = 0;

    for (int i = 0; i < num_datoteki; i++) {
        void *res;
        pthread_join(nitki[i], &res);
        if (res) {
            counts[i] = *(int *)res;
            free(res);
        } else {
            counts[i] = 0;
        }
        total_occurrences += counts[i];
    }

    // Print percentages after total is known
    for (int i = 0; i < num_datoteki; i++) {
        float pct = 0.0f;
        if (total_occurrences > 0) pct = ((float)counts[i] / (float)total_occurrences) * 100.0f;
        printf("Procent na pojavenost vo datotekata %s na zborot %s e %.2f%%\n",
               datoteki_ptrs[i], global_zbor, pct);
    }

    free(counts);
    return 0;
}