@echo off
setlocal

:: Change to project root directory (two levels up from this script)
cd /d "%~dp0..\.."

set "SC_EXE=build\bin\Debug\ShaderCompiler.exe"

if not exist "%SC_EXE%" (
    echo ShaderCompiler executable not found:
    echo   "%SC_EXE%"
    echo Please build the project first, e.g. run utility\build.bat.
    pause
    exit /b 1
)

echo Starting ShaderCompiler...
echo   Working directory: %CD%
echo   Executable       : "%SC_EXE%"
echo.

:: Forward any command-line arguments to ShaderCompiler.exe
"%SC_EXE%" %*

endlocal


