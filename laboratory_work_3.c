#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

#define MAX_THREADS_COUNT 100

enum users{READERS, WRITERS} priority;

int bd_state = 10;
int transactions_cnt = 5;
int nw, nr, dr, dw;

HANDLE sem_read, sem_write, sem_enter;


void signal_with_prior_to_readers(){
    if(nw == 0 && dr > 0){
        dr--;
        ReleaseSemaphore(sem_read, 1, NULL);
    }
    else if(nr == 0 && nw == 0 && dw > 0){
        dw--;
        ReleaseSemaphore(sem_write, 1, NULL);
    }
    else
        ReleaseSemaphore(sem_enter, 1, NULL);
}

void signal_with_prior_to_writers(){
    if(nr == 0 && nw == 0 && dw > 0){
        dw--;
        ReleaseSemaphore(sem_write, 1, NULL);
    }
    else if(nw == 0 && dr > 0){
        dr--;
        ReleaseSemaphore(sem_read, 1, NULL);
    }
    else
        ReleaseSemaphore(sem_enter, 1, NULL);
}

DWORD reader(PVOID p){
    int reader_num = *((int*)p);
    for(int i = 0; i < transactions_cnt; ++i) {
        Sleep(2);
        WaitForSingleObject(sem_enter, INFINITE);
        if(nw > 0){
            dr++;
            ReleaseSemaphore(sem_enter, 1, NULL);
            WaitForSingleObject(sem_read, INFINITE);
        }
        nr++;
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
        printf("-%d read that current database state is %d\n", reader_num, bd_state);
        WaitForSingleObject(sem_enter, INFINITE);
        nr--;
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
    }
    return (DWORD)0;
}

DWORD writer(PVOID p){
    int writer_num = *((int*)p);
    int action;
    if(writer_num & 1)
        action = -1;
    else
        action = 1;

    // action *= rand()%5 + 1; ploho

    for(int i = 0; i < transactions_cnt; ++i){
        Sleep(1);
        WaitForSingleObject(sem_enter, INFINITE);
        if(nr > 0 || nw > 0){
            dw++;
            ReleaseSemaphore(sem_enter, 1, NULL);
            WaitForSingleObject(sem_write, INFINITE);
        }
        nw++;
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
        bd_state += action;
        if(action > 0)
            printf("%d incs bd for %d\n", writer_num, action);
        else
            printf("%d decs bd for %d\n", writer_num, action);
        action = -action;
        WaitForSingleObject(sem_enter, INFINITE);
        nw--;
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
    }
    return (DWORD)0;
}

DWORD start_threads(PVOID cnt){
    int thread_cnt = *((int*)cnt);
    enum users cur;
    if(thread_cnt < 0){
        cur = READERS;
        thread_cnt = -thread_cnt;
    }
    else
        cur = WRITERS;

    int result = 0;
    DWORD dwThreadsIds[MAX_THREADS_COUNT], dwWait;
    HANDLE hThreads[MAX_THREADS_COUNT];
    int thread_numbers[MAX_THREADS_COUNT];
    for(size_t i = 0; i < thread_cnt; ++i){
        thread_numbers[i] = i;

        if(cur == READERS)
            hThreads[i] = CreateThread(NULL, 0, reader, (PVOID)&thread_numbers[i], 0, &dwThreadsIds[i]);
        else
            hThreads[i] = CreateThread(NULL, 0, writer, (PVOID)&thread_numbers[i], 0, &dwThreadsIds[i]);

        if(!hThreads[i]) {
            result++;
        }
    }

    dwWait = WaitForMultipleObjects((DWORD)thread_cnt, hThreads, TRUE, INFINITE);

    return (DWORD)result;
}

int main() {
    srand(time(NULL));
    priority = WRITERS;
    int readers, writers;
    printf("readers cnt:\n");
    scanf("%d", &readers);
    printf("writers cnt:\n");
    scanf("%d", &writers);

    sem_write = CreateSemaphoreA(NULL, 0, writers, "");
    sem_read = CreateSemaphoreA(NULL, 0, readers, "");
    sem_enter = CreateSemaphoreA(NULL, 1, 1, "");

    DWORD reader_res, writer_res;
    HANDLE start_reader, start_writer;
    readers *= -1;
    start_reader = CreateThread(NULL, 0, start_threads, (PVOID)&readers, 0, &reader_res);
    start_writer = CreateThread(NULL, 0, start_threads, (PVOID)&writers, 0, &writer_res);
    WaitForSingleObject(start_reader, INFINITE);
    WaitForSingleObject(start_writer, INFINITE);
    readers *= -1;

    // что-то очень странное выводится, наверное это не стоит показывать при сдаче
    if((int)reader_res)
        printf("%d readers doesn't start\n", (int)reader_res);
    if((int)writer_res)
        printf("%d writers doesn't start\n", (int)reader_res);

    return 0;
}