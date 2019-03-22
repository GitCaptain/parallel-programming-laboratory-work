#include <windows.h>

int main(int argc, char** argv) {
    HANDLE hDbFile, hDbFileMap, hProcdFile, hProcdFileMap;
    int *arr;

    hDbFile = CreateFile((LPCSTR)"database.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    hDbFileMap = CreateFileMapping(hDbFile, NULL, PAGE_READWRITE, 0, sizeof(int), (LPCSTR)"database.txt");

    arr = (int*)MapViewOfFile(hDbFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

    *arr = 10;

    UnmapViewOfFile((LPVOID)arr);
    CloseHandle(hDbFileMap);
    CloseHandle(hDbFile);

    hProcdFile = CreateFile((LPCSTR)"process_data.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
                            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    hProcdFileMap = CreateFileMapping(hProcdFile, NULL, PAGE_READWRITE, 0, 4*sizeof(int), (LPCSTR)"process_data.txt");

    arr = (int*)MapViewOfFile(hProcdFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 4*sizeof(int));
    for(int i = 0; i < 4; ++i)
        arr[i] = 0;

    UnmapViewOfFile((LPVOID)arr);
    CloseHandle(hProcdFileMap);
    CloseHandle(hProcdFile);
    return 0;
}