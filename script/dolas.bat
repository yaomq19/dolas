@echo off
setlocal

:: Change to project root directory
cd /d "%~dp0.."

:: Run the application
echo Starting Dolas...
build\bin\Debug\Dolas.exe
if %errorlevel% neq 0 (
    echo Program exited with error code: %errorlevel%
    pause
    exit /b %errorlevel%
)

pause
endlocal

