@echo off
setlocal

git submodule update --init --recursive

rd /s /q .\build
mkdir build
cd .\build
cmake .. -G "Visual Studio 17 2022" -A x64

endlocal
pause >nul

pause >nul