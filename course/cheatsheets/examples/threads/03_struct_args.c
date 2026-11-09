// threads.md §2 — several arguments: one struct per thread, kept alive until join
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct Job {
    int id;
    const char *word;
    int result;                     // the thread writes its answer here
};

void *work(void *arg)
{
    struct Job *j = arg;
    printf("job %d got word '%s' (pthread_self ends in ...%lu)\n", j->id, j->word,
           (unsigned long)pthread_self() % 1000);
    j->result = j->id * 10;
    return NULL;
}

int main(void)
{
    const char *words[3] = {"alfa", "beta", "gama"};
    struct Job jobs[3];             // lives in main until after the joins: safe
    pthread_t tid[3];
    for (int i = 0; i < 3; i++) {
        jobs[i].id = i;
        jobs[i].word = words[i];
        pthread_create(&tid[i], NULL, work, &jobs[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(tid[i], NULL);
        printf("main: job %d result %d\n", i, jobs[i].result);
    }
    return 0;
}
