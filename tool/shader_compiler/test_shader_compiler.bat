@echo off
echo ====================================================
echo    测试 Dolas Shader Compiler
echo ====================================================
echo.

cd /d "%~dp0"

REM 检查编译器是否存在
if not exist "build\bin\ShaderCompiler.exe" (
    echo 错误: ShaderCompiler.exe 不存在，请先编译项目
    echo 运行以下命令编译:
    echo   mkdir build
    echo   cd build
    echo   cmake ..
    echo   cmake --build . --config Release
    pause
    exit /b 1
)

echo 1. 测试编译所有shader文件
echo ====================================================
"build\bin\ShaderCompiler.exe"

echo.
echo.
echo 2. 测试编译指定文件 (如果存在)
echo ====================================================
if exist "..\content\shader\deferred_shading.ps.hlsl" (
    "build\bin\ShaderCompiler.exe" deferred_shading.ps.hlsl
) else (
    echo 指定的测试文件不存在，跳过测试
)

echo.
echo ====================================================
echo 测试完成！查看 build\logs\ 目录下的日志文件
echo ====================================================
pause
