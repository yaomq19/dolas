@echo off
REM DDS Texture Generator batch script
REM Convert images in content/textures directory to DDS format

echo ========================================
echo DDS Texture Generator - Dolas Engine
echo ========================================

REM Switch to project root directory
cd /d "%~dp0\.."

REM Run Python script
echo Running DDSGenerate.py...
python script/DDSGenerate.py %*

REM Show results
if %ERRORLEVEL% EQU 0 (
    echo.
) else (
    echo.
    echo Please check error messages and try again.
)

echo.
echo Press any key to continue...
pause > nul 