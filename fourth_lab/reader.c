#include <stdio.h>
#include <windows.h>
#include <conio.h>

enum users{READERS, WRITERS} priority;

int transactions_cnt = 5;
int *bd_state, *proc_data, *nw, *nr, *dr, *dw;

HANDLE sem_read, sem_write, sem_enter;


void signal_with_prior_to_readers(){
    if(*nw == 0 && *dr > 0){
        (*dr)--;
        FlushViewOfFile((LPCVOID)dr, sizeof(int));
        ReleaseSemaphore(sem_read, 1, NULL);
    }
    else if(*nr == 0 && *nw == 0 && *dw > 0){
        (*dw)--;
        FlushViewOfFile((LPCVOID)dw, sizeof(int));
        ReleaseSemaphore(sem_write, 1, NULL);
    }
    else
        ReleaseSemaphore(sem_enter, 1, NULL);
}

void signal_with_prior_to_writers(){
    if(*nr == 0 && *nw == 0 && *dw > 0){
        (*dw)--;
        FlushViewOfFile((LPCVOID)dw, sizeof(int));
        ReleaseSemaphore(sem_write, 1, NULL);
    }
    else if(*nw == 0 && *dr > 0){
        (*dr)--;
        FlushViewOfFile((LPCVOID)dr, sizeof(int));
        ReleaseSemaphore(sem_read, 1, NULL);
    }
    else
        ReleaseSemaphore(sem_enter, 1, NULL);
}

int reader(int reader_num){
    for(int i = 0; i < transactions_cnt; ++i) {
        Sleep(2);
        WaitForSingleObject(sem_enter, INFINITE);
        if(*nw > 0){
            (*dr)++;
            FlushViewOfFile((LPCVOID)dr, sizeof(int));
            ReleaseSemaphore(sem_enter, 1, NULL);
            WaitForSingleObject(sem_read, INFINITE);
        }
        (*nr)++;
        FlushViewOfFile((LPCVOID)nr, sizeof(int));
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
        printf("%d read that current database state is %d\n", reader_num, *bd_state);
        WaitForSingleObject(sem_enter, INFINITE);
        (*nr)--;
        FlushViewOfFile((LPCVOID)nr, sizeof(int));
        if(priority == READERS)
            signal_with_prior_to_readers();
        else
            signal_with_prior_to_writers();
    }
    return 0;
}

int main(int argc, char** argv) {
    priority = READERS;
    HANDLE hDatabase, hProcessData, hDatabaseMap, hProcessDataMap;
    printf("reader %s, readers_cnt %s\n", argv[2], argv[1]);
    int readers = atoi(argv[1]), num = atoi(argv[2]);
    sem_write = CreateSemaphoreA(NULL, 0, 1, "w");
    sem_read = CreateSemaphoreA(NULL, 0, readers, "r");
    sem_enter = CreateSemaphoreA(NULL, 1, 1, "e");

    hDatabase = CreateFile((LPCSTR)"database.txt", GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    hProcessData = CreateFile((LPCSTR)"process_data.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                           NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hDatabase == INVALID_HANDLE_VALUE) {
        printf("Process reader %d: cant open database\n", num);
        return 0;
    }
    if(hProcessData == INVALID_HANDLE_VALUE) {
        printf("Process reader %d: cant open process data\n", num);
        return 0;
    }

    hDatabaseMap = CreateFileMapping(hDatabase, NULL, PAGE_READONLY, 0, sizeof(int), (LPCSTR)"database.txt");
    Sleep(1000);
    hProcessDataMap = CreateFileMapping(hProcessData, NULL, PAGE_READWRITE, 0, 4*sizeof(int), (LPCSTR)"process_data.txt");
    Sleep(1000);

    bd_state = (int*)MapViewOfFile(hDatabaseMap, FILE_MAP_READ, 0, 0, sizeof(int));
    proc_data = (int*)MapViewOfFile(hProcessDataMap, FILE_MAP_ALL_ACCESS , 0, 0, 4*sizeof(int));
    nw = &proc_data[0], nr = &proc_data[1], dw = &proc_data[2], dr = &proc_data[3];


    printf("%d ", *bd_state);
    for(int i = 0; i < 4; ++i) {
        printf("%d ", proc_data[i]);
    }
    printf("\n");

    reader(num);

    UnmapViewOfFile((LPVOID)proc_data);
    UnmapViewOfFile((LPVOID)bd_state);

    CloseHandle(hDatabaseMap);
    CloseHandle(hDatabase);

    CloseHandle(sem_enter);
    CloseHandle(sem_read);
    CloseHandle(sem_write);
    printf("press any key for exit\n");
    getch();
    return 0;
}