#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define QUEUE_SIZE_4MB (4 * 1024 * 1024 / PAGE_SIZE)
#define QUEUE_SIZE_16MB (16 * 1024 * 1024 / PAGE_SIZE)

typedef struct Page {
    int page_number;
    int access_count;
    int in_active_list;
    struct Page* next;
    struct Page* prev;
} Page;

typedef struct Queue {
    int size_limit;
    int current_size;
    int total_hits;
    int total_accesses;
    Page* head;
    Page* tail;
} Queue;

Queue* initializeQueue(int size_limit) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        fprintf(stderr, "메모리 할당 오류\n");
        exit(1);
    }
    queue->size_limit = size_limit;
    queue->current_size = 0;
    queue->total_hits = 0;
    queue->total_accesses = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void insertPage(Queue* queue, int page_number) {
    if (queue->current_size >= queue->size_limit) {
        if (queue->tail != NULL) {
            Page* evicted_page = queue->tail;
            queue->tail = evicted_page->prev;
            if (queue->tail != NULL) {
                queue->tail->next = NULL;
            }
            free(evicted_page);
            queue->current_size--;
        }
    }

    Page* new_page = (Page*)malloc(sizeof(Page));
    if (new_page == NULL) {
        fprintf(stderr, "메모리 할당 오류\n");
        exit(1);
    }
    new_page->page_number = page_number;
    new_page->access_count = 1;
    new_page->in_active_list = 1;
    new_page->next = queue->head;
    new_page->prev = NULL;

    if (queue->head != NULL) {
        queue->head->prev = new_page;
    }

    queue->head = new_page;

    if (queue->tail == NULL) {
        queue->tail = new_page;
    }

    queue->current_size++;
}

void accessPage(Queue* queue, int page_number) {
    Page* current_page = queue->head;

    while (current_page != NULL) {
        if (current_page->page_number == page_number) {
            current_page->access_count++;
            current_page->in_active_list = 1;
            queue->total_hits++;
            return;
        }
        current_page = current_page->next;
    }

    insertPage(queue, page_number);
}

void freeQueue(Queue* queue) {
    Page* current_page = queue->head;
    while (current_page != NULL) {
        Page* next_page = current_page->next;
        free(current_page);
        current_page = next_page;
    }
    free(queue);
}

int main() {
    FILE* file = fopen("w3_mixed.txt", "r");
    if (file == NULL) {
        perror("파일 열기 오류");
        return 1;
    }

    Queue* queue_4mb = initializeQueue(QUEUE_SIZE_4MB);
    Queue* queue_16mb = initializeQueue(QUEUE_SIZE_16MB);

    char operation;
    int page_number;

    while (fscanf(file, "%c %d\n", &operation, &page_number) != EOF) {
        queue_4mb->total_accesses++;
        queue_16mb->total_accesses++;

        if (operation == 'R') {
            accessPage(queue_4mb, page_number);
            accessPage(queue_16mb, page_number);
        }
        else if (operation == 'W') {
            // 필요한 경우 쓰기 작업 처리
        }
    }

    double hit_ratio_4mb = (double)queue_4mb->total_hits / queue_4mb->total_accesses;
    double hit_ratio_16mb = (double)queue_16mb->total_hits / queue_16mb->total_accesses;

    // 결과 출력
    printf("4MB HIT-RATIO: %.4f\n", hit_ratio_4mb);
    printf("16MB HIT-RATIO: %.4f\n", hit_ratio_16mb);

    // 할당된 메모리 해제
    freeQueue(queue_4mb);
    freeQueue(queue_16mb);

    // 파일 닫기
    fclose(file);

    return 0;
}