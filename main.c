#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#define BLOCK_SIZE 4096 // 4 Кбайта
#define NUM_BLOCKS 16   // 16 подблоков (64 Кбайта)
#define NUM_THREADS 3    // 3 потока

typedef struct {
    int *data;
    pthread_mutex_t *mutex;
    int index;
} Block;

Block blocks[NUM_BLOCKS];

void* generate_block(void* arg) {
    int index = *(int*)arg;
    free(arg); // освобождение памяти

    while (1) {
        pthread_mutex_lock(&blocks[index].mutex[index]);
        for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
            blocks[index].data[i] = rand() % 100; // Генерируем случайные числа
        }
        pthread_mutex_unlock(&blocks[index].mutex[index]);
        sleep(1); // Имитация времени работы
    }
    return NULL;
}

void* sort_block(void* arg) {
    int index = *(int*)arg;
    free(arg);

    while (1) {
        pthread_mutex_lock(&blocks[index].mutex[index]);
        // Сортировка подблока
        qsort(blocks[index].data, BLOCK_SIZE / sizeof(int), sizeof(int), compare);
        pthread_mutex_unlock(&blocks[index].mutex[index]);
        sleep(1); // Имитация времени работы
    }
    return NULL;
}

void* print_block(void* arg) {
    int index = *(int*)arg;
    free(arg);

    while (1) {
        pthread_mutex_lock(&blocks[index].mutex[index]);
        // Вывод содержимого подблока
        for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
            printf("%d ", blocks[index].data[i]);
        }
        printf("\n");
        pthread_mutex_unlock(&blocks[index].mutex[index]);
        sleep(1); // Имитация времени работы
    }
    return NULL;
}

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int main() {
    srand(time(NULL));
    
    // Создание разделяемой памяти
    for (int i = 0; i < NUM_BLOCKS; i++) {
        blocks[i].data = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        blocks[i].mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(&blocks[i].mutex[i], NULL);
        blocks[i].index = i;
    }

    pthread_t threads[NUM_THREADS][NUM_BLOCKS];

    // Создание потоков
    for (int i = 0; i < NUM_BLOCKS; i++) {
        int* index = malloc(sizeof(int));
        *index = i;
        pthread_create(&threads[0][i], NULL, generate_block, index);

        index = malloc(sizeof(int));
        *index = i;
        pthread_create(&threads[1][i], NULL, sort_block, index);

        index = malloc(sizeof(int));
        *index = i;
        pthread_create(&threads[2][i], NULL, print_block, index);
    }

    // Ожидание завершения потоков (в данном случае они бесконечные)
    for (int i = 0; i < NUM_BLOCKS; i++) {
        pthread_join(threads[0][i], NULL);
        pthread_join(threads[1][i], NULL);
        pthread_join(threads[2][i], NULL);
    }

    for (int i = 0; i < NUM_BLOCKS; i++) {
        munmap(blocks[i].data, BLOCK_SIZE);
        pthread_mutex_destroy(&blocks[i].mutex[i]);
        free(blocks[i].mutex);
    }

    return 0;
}