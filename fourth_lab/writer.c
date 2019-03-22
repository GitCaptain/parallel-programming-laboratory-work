#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <stdlib.h>

#define MAX_THREADS_COUNT 100

enum users{READERS, WRITERS} priority;

int transactions_cnt = 5;
int nw, nr, dr, dw, bd_state;

HANDLE sem_read, sem_write, sem_enter;


void signal_with_prior_to_readers(){
    if(nw == 0 && dr > 0){
        dr--;
        FlushViewOfFile(dr, sizeof(int));
        ReleaseSemaphore(sem_read, 1, NULL);
    }
    else if(nr == 0 && nw == 0 && dw > 0){
        dw--;
        FlushViewOfFile(dw, sizeof(int));
        ReleaseSemaphore(sem_write, 1, NULL);
    }
    else
        ReleaseSemaphore(sem_enter, 1, NULL);
}

void signal_with_prior_to_writers(){
    if(nr == 0 && nw == 0 && dw > 0){
        dw--;
        FlushViewOfFile(dw, sizeof(int));
        ReleaseSemaphore(sem_write, 1, NULL);
    }
    else if(nw == 0 && dr > 0){
        dr--;
        FlushViewOfFile(dr, sizeof(int));
        ReleaseSemaphore(sem_read, 1, NULL);
    }
    else
        ReleaseSemaphore(sem_enter, 1, NULL);
}

int writer(int writer_num){
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
            FlushViewOfFile(dw, sizeof(int));
            ReleaseSemaphore(sem_enter, 1, NULL);
            WaitForSingleObject(sem_write, INFINITE);
        }
        nw++;
        FlushViewOfFile(nw, sizeof(int));
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
        bd_state += action;
        FlushViewOfFile(bd_state, sizeof(int));
        if(action > 0)
            printf("%d incs bd for %d\n", writer_num, action);
        else
            printf("%d decs bd for %d\n", writer_num, action);
        action = -action;
        WaitForSingleObject(sem_enter, INFINITE);
        nw--;
        FlushViewOfFile(nw, sizeof(int));
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
    }
    return 0;
}

int main(int argc, char** argv) {
    priority = WRITERS;
    HANDLE hFile, hFileMap[5];
    sem_write = CreateSemaphoreA(NULL, 0, 1, "w");
    sem_read = CreateSemaphoreA(NULL, 0, atoi(argv[1]), "r");
    sem_enter = CreateSemaphoreA(NULL, 1, 1, "e");

    hFile = CreateFile(L"data base.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    hFileMap[0] = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 3, NULL);
    hFileMap[1] = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 4, 7, NULL);
    hFileMap[2] = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 8, 11, NULL);
    hFileMap[3] = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 12, 15, NULL);
    hFileMap[4] = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 16, 19, NULL);
    Sleep(1000);
    bd_state = (int)MapViewOfFile(hFileMap[0], FILE_MAP_WRITE, 0, 3, 4);
    nr = (int)MapViewOfFile(hFileMap[1], FILE_MAP_WRITE, 4, 7, 4);
    nw = (int)MapViewOfFile(hFileMap[2], FILE_MAP_WRITE, 8, 11, 4);
    dr = (int)MapViewOfFile(hFileMap[3], FILE_MAP_WRITE, 12, 15, 4);
    dw = (int)MapViewOfFile(hFileMap[4], FILE_MAP_WRITE, 16, 19, 4);
    reader(atoi(argv[2]));
    UnmapViewOfFile(bd_state);
    UnmapViewOfFile(nr);
    UnmapViewOfFile(nw);
    UnmapViewOfFile(dr);
    UnmapViewOfFile(dw);
    CloseHandle(hFileMap);
    CloseHandle(hFile);
    return 0;
}