@echo off
title process start

set /p readers="enter readers cnt:"

set /p writers="enter writers cnt:"

cd D:\Projects\c++\parallel_programming_4
cd cmake-build-debug

echo preparing data base and process data, press any key, when done.
start db_create.exe
Pause

for /l %%i in (1, 1, %readers%) do (
    echo start readers.exe %readers% %%i
    start reader.exe %readers% %%i
    echo reader started
)		

rem передаем во writers.exe параметр readers, так как именно он нужен для семафора
for /l %%i in (1, 1, %writers%) do (
    echo start writers.exe %readers% %%i
    start writer.exe %readers% %%i
    echo writer started
)

Pause

