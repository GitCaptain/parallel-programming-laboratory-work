#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <string.h>

#define MAX_BUF_SIZE 100
#define MAX_THREADS_COUNT 100

char a[MAX_BUF_SIZE], b[MAX_BUF_SIZE];
unsigned int div_r, mod_r;

DWORD WINAPI Check(PVOID p){
    unsigned int cur_number = *((unsigned int*)p);
    unsigned int result = 1;
    unsigned int start, stop;
    if(cur_number < mod_r){ // нужно выполнить на одну проверку больше
        start = div_r * cur_number;
        stop = start + div_r;
    } else{
        start = div_r * (mod_r) + (div_r-1) * (cur_number-mod_r);
        stop = start + div_r - 1;
    }
    if(stop > MAX_BUF_SIZE) {
        return (DWORD) result;
    }
    printf("thread %d: start = %d, stop = %d\n", cur_number, start, stop);
    for(size_t i = start; i < stop; ++i){
        if(a[i] != b[i])
            result = 0;
    }
    return (DWORD)result;
}

int main(int argc, char **argv) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    unsigned int threads_cnt;
    printf("Enter threads count: ");
    scanf("%d", &threads_cnt);

    printf("\nEnter first string, which length must not be greater than %d symbols:\n", MAX_BUF_SIZE);
    fflush(stdin);
    fgets(a, MAX_BUF_SIZE, stdin);
    printf("%s\n", a);

    printf("\nEnter second string, which length must not be greater than %d symbols\n", MAX_BUF_SIZE);
    fflush(stdin);
    fgets(b, MAX_BUF_SIZE, stdin);
    printf("%s\n", a);

    unsigned int length = strlen(a), length2 = strlen(b);
    if(length != length2){
        printf("Strings are different\n");
        return 0;
    }

    unsigned int thread_numbers[MAX_THREADS_COUNT];
    DWORD dwThreadsIds[MAX_THREADS_COUNT], dwWait, dwResults[MAX_THREADS_COUNT];
    HANDLE hThreads[MAX_THREADS_COUNT];
    div_r = (length + threads_cnt - 1) / threads_cnt, mod_r = length % threads_cnt;
    for(size_t i = 0; i < threads_cnt; ++i){
        thread_numbers[i] = i;
        hThreads[i] = CreateThread(NULL, 0, Check, (PVOID)&thread_numbers[i], 0, &dwThreadsIds[i]);

        if(!hThreads[i])
            printf("main process: thread %d not execute!\n", i);

    }

    dwWait = WaitForMultipleObjects(threads_cnt, hThreads, TRUE, INFINITE);

    int equal = 1;
    for(int i = 0; i < threads_cnt; ++i) {
        GetExitCodeThread(hThreads[i], &dwResults[i]);
        printf("thread %d ended with result: %d\n", i, (int)dwResults[i]);
        equal &= (int)dwResults[i];
    }


    if(equal)
        printf("Strings are equal\n");
    else
        printf("Strings are different\n");

    return 0;
}