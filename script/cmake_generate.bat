@echo off
setlocal

::: 切到项目根目录
cd /d "%~dp0..\.."

::: 检查 VCPKG_ROOT 是否设置
if "%VCPKG_ROOT%"=="" (
    echo [ERROR] The VCPKG_ROOT environment variable is not set!
    echo Please first point VCPKG_ROOT to your vcpkg directory, for example:
    echo   setx VCPKG_ROOT "D:\Program Files\vcpkg\vcpkg-2025.10.17"
    pause
    exit /b 1
)

::: 自动选择最高可用的 Visual Studio 版本（從 2026 往下到 2010）
set "GEN="

call :TryGenerator "Visual Studio 18 2026"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 17 2022"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 16 2019"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 15 2017"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 14 2015"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 12 2013"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 11 2012"
if defined GEN goto GotGenerator
call :TryGenerator "Visual Studio 10 2010"

if not defined GEN (
    echo [ERROR] No available Visual Studio CMake generator was detected.
    echo Please ensure that you have Visual Studio 2010 or later installed, and that your CMake version supports the corresponding generator name.
    pause
    exit /b 1
)

:GotGenerator
echo Using CMake generators: %GEN%
echo Configuring project without presets...

cmake -S . -B build -G "%GEN%" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DVCPKG_INSTALLED_DIR="%cd%\build\vcpkg_installed" ^
    -DVCPKG_TARGET_TRIPLET=x64-windows ^
    -DVCPKG_MANIFEST_MODE=ON

if %errorlevel% neq 0 (
    echo Configuration failed!
    pause
    exit /b %errorlevel%
)

echo Configuration completed successfully!
pause
endlocal
exit /b 0

::: 子函数：尝试某个 VS 生成器是否可用（在临时目录里做一次空配置）
:TryGenerator
set "TMPDIR=%TEMP%\cmake_gen_test_%RANDOM%"
cmake -S . -B "%TMPDIR%" -G "%~1" -N >nul 2>&1
if %errorlevel%==0 (
    set "GEN=%~1"
)
if exist "%TMPDIR%" rd /s /q "%TMPDIR%" >nul 2>&1
exit /b 0