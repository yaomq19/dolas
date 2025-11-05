@echo off
REM Batch script to compile HLSL shaders using CompileShader.py
REM Usage: compile_shaders.bat [--verbose]

echo Dolas Engine - HLSL Shader Compiler
echo =====================================

REM 检查Python是否可用
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found! Please install Python 3.6 or later.
    pause
    exit /b 1
)

REM 切换到项目根目录
cd /d "%~dp0\.."

REM 运行Python脚本
echo Running CompileShaders.py...
python script/CompileShaders.py %*

REM 显示结果
if errorlevel 1 (
    echo.
    echo FAILED: Some shaders failed to compile.
    pause
    exit /b 1
) else (
    echo.
    echo SUCCESS: All shaders compiled successfully!
    if "%1"=="" pause
)

exit /b 0 