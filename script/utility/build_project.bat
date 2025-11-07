@echo off
setlocal

:: Change to project root directory
cd /d "%~dp0..\.."

:: Build project
echo Building project...
cmake --build build
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build completed successfully!
pause
endlocal

