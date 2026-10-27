@echo off
rem Compiles and runs one Ry program:  run.cmd examples\1.ry
rem Ry -> C (main.py), C -> .exe (gcc), then runs the .exe.
if "%~1"=="" (echo usage: run.cmd file.ry & exit /b 1)
python "%~dp0main.py" "%~1" || exit /b 1
gcc -std=c11 -o "%~dpn1.exe" "%~dpn1.c" || exit /b 1
"%~dpn1.exe"
