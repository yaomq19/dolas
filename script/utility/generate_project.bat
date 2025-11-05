@echo off
setlocal

:: Change to project root directory
cd /d "%~dp0..\.."

:: Configure using preset
echo Configuring with preset: default
cmake --preset default
if %errorlevel% neq 0 (
    echo Configuration failed!
    exit /b %errorlevel%
)

echo Configuration completed successfully!
pause
endlocal

