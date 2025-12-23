#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>

#define FILE_SIZE 1000

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;

size_t globalOffset = 0;   
size_t realSize = 0;      
bool finished = false;     

void *ReadFromFile(void *arg) {
    char *mem = (char*) arg;

    srand((unsigned int)(time(NULL) ^ (unsigned long)pthread_self()));

    while (1) {
        pthread_mutex_lock(&fileMutex);

        
        if (finished) {
            pthread_mutex_unlock(&fileMutex);
            break;
        }

        
        if (globalOffset >= realSize) {
            finished = true;
            pthread_mutex_unlock(&fileMutex);
            break;
        }

        int X = rand() % 90 + 10;  
        size_t remaining = realSize - globalOffset;
        size_t toRead = (remaining < (size_t)X) ? remaining : (size_t)X;

        
        fwrite(mem + globalOffset, 1, toRead, stdout);
        fflush(stdout);

        globalOffset += toRead;

        if (globalOffset >= realSize) {
            finished = true;
        }

        pthread_mutex_unlock(&fileMutex);

        sleep(2);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int numThreads = 0;

    if (argc < 2) {
        printf("insufficient arguments\n");
        return 1;
    }

    char *filename = argv[1];

    printf("Enter the number of threads: \n");
    scanf("%d", &numThreads);

    pthread_t threads[numThreads];

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error fstat");
        close(fd);
        return 1;
    }

    realSize = (size_t)st.st_size;
    if (realSize > FILE_SIZE) realSize = FILE_SIZE;

    if (realSize == 0) {
        printf("(Empty file)\n");
        close(fd);
        return 0;
    }

    char *mapped = mmap(NULL, realSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(fd);
        return 1;
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], NULL, ReadFromFile, (void*)mapped);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n");

    munmap(mapped, realSize);
    close(fd);
    pthread_mutex_destroy(&fileMutex);

    return 0;
}
