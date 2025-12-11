@echo off

rem ------------------------------------------------------------------------------
rem Main loop over ../test/*.c
rem ------------------------------------------------------------------------------

for %%F in (".\test\*.c") do (
    call :testcase "%%F"
)

exit /b 0

rem ------------------------------------------------------------------------------
rem :testcase label acts like the bash function testcase()
rem Usage: call :testcase "..\test\file.c"
rem ------------------------------------------------------------------------------

:testcase
set "file=%~1"
set "name=%~n1"

echo Running test [%name%]...

echo Compiling %file% with gcc
gcc "%file%"
if errorlevel 1 exit /b 1

echo Compiling %file% with c.c
.\c.c.exe "%file%" ".\test\%name%.c" > "%name%.actual"

rem Run the produced executable with gcc
.\a "..\test\%name%.c" > "%name%.expect"

rem Compare outputs (using fc instead of diff)
fc /N /W "%name%.expect" "%name%.actual" >nul
if errorlevel 1 (
    echo ERROR: Test [%name%] failed.
    exit /b 1
) else (
    echo Test [%name%] passed.
)

goto :eof