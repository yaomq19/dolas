@echo off
setlocal

::: Change to project root directory
cd /d "%~dp0..\.."

echo Cleaning CMake cache in "build" directory...

if not exist build (
    echo The build directory does not exist and does not need to be cleaned.
    pause
    exit /b 0
)

if exist build\CMakeCache.txt (
    del /f /q build\CMakeCache.txt
)

if exist build\CMakeFiles (
    rmdir /s /q build\CMakeFiles
)

if exist build\cmake_install.cmake (
    del /f /q build\cmake_install.cmake
)

echo The CMake cache has been cleared. You can rerun cmake_generate.bat to configure it.
pause
endlocal


