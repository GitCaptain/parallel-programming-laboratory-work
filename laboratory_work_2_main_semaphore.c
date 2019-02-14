#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS_COUNT 100

int valentines[MAX_THREADS_COUNT], luckers[MAX_THREADS_COUNT];
unsigned int threads_cnt, year = 0;
HANDLE arrive_sem, continue_sem;

DWORD WINAPI serverTyan(PVOID p){

    while(year < 3){
        int wait_arrive = 0;
        while(wait_arrive < threads_cnt-1){
            ReleaseSemaphore(arrive_sem, 0, &wait_arrive);
        }

        int lucky_guy = 0;
        for(size_t i = 1; i < threads_cnt; ++i){
            luckers[i] = 0;
            if(valentines[i] > valentines[lucky_guy])
                lucky_guy = i;
        }

        luckers[lucky_guy] = 1;
        year++;

        ReleaseSemaphore(continue_sem, threads_cnt-1, NULL);
    }
    return (DWORD)0;
}

DWORD WINAPI clientKun(PVOID p){
    unsigned int index = *((unsigned int*)p);
    int lucky = 0;
    int continue_sem = 0;
    while(year < 3){
        valentines[index] = (rand() % (100 * index)) * rand(); // придумать ченить нормальное

        ReleaseSemaphore(arrive_sem, 1, NULL);

        while(continue_sem < threads_cnt-1){
            ReleaseSemaphore(continue_sem, 0, &continue_sem);
        };

        if(luckers[index]){
            lucky = 1;
            printf("Lucky guy number %d!\n", index);
        }

        ReleaseSemaphore(continue_sem, -1, NULL);
    }
    return (DWORD)lucky;
}



int main(int argc, char **argv) {
    srand(time(NULL));
    printf("Enter threads count: ");
    scanf("%d", &threads_cnt);
    threads_cnt++; // Отдельный тред для сервер тян

    unsigned int thread_numbers[MAX_THREADS_COUNT];
    DWORD dwThreadsIds[MAX_THREADS_COUNT], dwWait, dwResults[MAX_THREADS_COUNT];
    HANDLE hThreads[MAX_THREADS_COUNT];

    CreateSemaphoreA(arrive_sem, 0, threads_cnt-1, NULL);
    CreateSemaphoreA(continue_sem, 0, threads_cnt-1, NULL);

    for(size_t i = 0; i < threads_cnt; ++i){
        thread_numbers[i] = i;

        if(i)
            hThreads[i] = CreateThread(NULL, 0, clientKun, (PVOID)&thread_numbers[i], 0, &dwThreadsIds[i]);
        else
            hThreads[i] = CreateThread(NULL, 0, serverTyan, (PVOID)&thread_numbers[i], 0, &dwThreadsIds[i]);

        if(!hThreads[i])
            printf("main process: thread %d not execute!\n", i);

    }

    dwWait = WaitForMultipleObjects(threads_cnt, hThreads, TRUE, INFINITE);

    for(size_t i = 1; i < threads_cnt; ++i) {
        GetExitCodeThread(hThreads[i], &dwResults[i]);
        if((int)dwResults[i])
            printf("thread %d was lucky.\n", i);
        else
            printf("thread %d was not lucky.\n", i);
    }


    return 0;
}