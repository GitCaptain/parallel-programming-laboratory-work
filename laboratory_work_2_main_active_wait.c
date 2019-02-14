#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_THREADS_COUNT 100

int arrive[MAX_THREADS_COUNT], _continue[MAX_THREADS_COUNT], valentines[MAX_THREADS_COUNT], luckers[MAX_THREADS_COUNT];
unsigned int threads_cnt, year = 0;

DWORD WINAPI serverTyan(PVOID p){
    while(year < 3){

        for(size_t i = 1; i < threads_cnt; ++i){
            while(!arrive[i]);
            arrive[i] = 0;
        }

        int lucky_guy = 0;
        for(size_t i = 1; i < threads_cnt; ++i){
            luckers[i] = 0;
            if(valentines[i] > valentines[lucky_guy])
                lucky_guy = i;
        }

        luckers[lucky_guy] = 1;
        year++;
        for(int i = 1; i < threads_cnt; ++i){
            _continue[i] = 1;
        }
    }
    return (DWORD)0;
}

DWORD WINAPI clientKun(PVOID p){
    unsigned int index = *((unsigned int*)p);
    int lucky = 0;
    while(year < 3){
        valentines[index] = (rand() % (100 * index)) * rand(); // придумать ченить нормальное
        arrive[index] = 1;
        while(!_continue[index]);
        if(luckers[index]){
            lucky = 1;
            printf("Lucky guy number %d!\n", index);
        }
        _continue[index] = 0;
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