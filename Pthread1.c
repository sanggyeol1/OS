#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>

#define MAX_BUFFER_SIZE 15
#define TOTAL_THREADS 5 // Added one more thread for the timer
#define LOOPS 50
#define SHUTDOWN_TIME 3 //시간 정의

HANDLE threads[TOTAL_THREADS];
CRITICAL_SECTION mutex;
CONDITION_VARIABLE cond;
int buffer[MAX_BUFFER_SIZE];
int count = 0;
int g_shutdown = 0; // 변경된 부분

void put(char item) {
    if (count < MAX_BUFFER_SIZE) {
        buffer[count++] = item;
    }
}

char get() {
    char item = ' ';
    if (count > 0) {
        item = buffer[--count];
    }
    return item;
}

DWORD WINAPI producerA(LPVOID arg) {
    while (!g_shutdown) {
        for (int c = 65; c <= 90; ++c) {
            EnterCriticalSection(&mutex);
            put(c);
            WakeConditionVariable(&cond);
            LeaveCriticalSection(&mutex);
        }
    }
    return 0;
}

DWORD WINAPI producerB(LPVOID arg) {
    while (!g_shutdown) {
        for (int c = 97; c <= 122; ++c) {
            EnterCriticalSection(&mutex);
            put(c);
            WakeConditionVariable(&cond);
            LeaveCriticalSection(&mutex);
        }
    }
    return 0;
}

DWORD WINAPI producerC(LPVOID arg) {
    while (!g_shutdown) {
        for (int num = 1; num <= 50; ++num) {
            EnterCriticalSection(&mutex);
            put(num);
            WakeConditionVariable(&cond);
            LeaveCriticalSection(&mutex);
        }
    }
    return 0;
}

DWORD WINAPI consumer(LPVOID arg) {
    int AtoZ_count = 0, atoz_count = 0, oneTo50_count = 0;

    while (!g_shutdown) {
        EnterCriticalSection(&mutex);
        while (count == 0 && !g_shutdown) {
            SleepConditionVariableCS(&cond, &mutex, INFINITE);
        }

        if (count > 0) {
            char item = get();
            WakeConditionVariable(&cond);
            LeaveCriticalSection(&mutex);

            if (item >= 65 && item <= 90) {
                AtoZ_count++;
            }
            else if (item >= 97 && item <= 122) {
                atoz_count++;
            }
            else if (item >= 1 && item <= 50) {
                oneTo50_count++;
            }
            if (item >= 1 && item <= 50)
                printf("%d\t", item);
            else
                printf("%c\t", item);

            int Full_count = AtoZ_count + atoz_count + oneTo50_count;
            if (Full_count % 10 == 0) {
                printf("\n");
            }
        }
        else {
            LeaveCriticalSection(&mutex);
        }
    }

    printf("\n\nAtoZ: %d\natoz: %d\n1to50: %d\n", AtoZ_count, atoz_count, oneTo50_count);

    return 0;
}

DWORD WINAPI timerThread(LPVOID arg) {
    Sleep(SHUTDOWN_TIME * 1000); // Sleep for 5 minutes
    g_shutdown = 1;

    printf("Auto-shutdown after 5 minutes.\n");

    return 0;
}

int main() {
    InitializeCriticalSection(&mutex);
    InitializeConditionVariable(&cond);

    
    threads[0] = CreateThread(NULL, 0, producerA, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, producerB, NULL, 0, NULL);
    threads[2] = CreateThread(NULL, 0, producerC, NULL, 0, NULL);
    threads[3] = CreateThread(NULL, 0, consumer, NULL, 0, NULL);
    threads[4] = CreateThread(NULL, 0, timerThread, NULL, 0, NULL); // Timer thread
    

    WaitForMultipleObjects(TOTAL_THREADS, threads, TRUE, INFINITE);

    DeleteCriticalSection(&mutex);

    return 0;
}