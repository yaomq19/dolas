@echo off
echo 正在从 DLL 生成导入库...

REM 设置 Visual Studio 环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
if errorlevel 1 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
)

REM 检查 DLL 是否存在
if not exist "assimp-vc143-mtd.dll" (
    echo 错误: 找不到 assimp-vc143-mtd.dll
    pause
    exit /b 1
)

REM 生成导出列表
echo 正在分析 DLL 导出符号...
dumpbin /exports assimp-vc143-mtd.dll > exports.txt

REM 创建 .def 文件
echo EXPORTS > assimp.def
for /f "skip=19 tokens=4" %%i in (exports.txt) do (
    if not "%%i"=="" echo %%i >> assimp.def
)

REM 生成导入库
echo 正在生成导入库...
lib /def:assimp.def /out:assimp-vc143-mtd.lib /machine:x64

REM 清理临时文件
del exports.txt
del assimp.exp

if exist "assimp-vc143-mtd.lib" (
    echo 成功生成导入库: assimp-vc143-mtd.lib
) else (
    echo 错误: 导入库生成失败
)

pause
