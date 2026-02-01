@echo off
if not exist build (
    mkdir build
)
cmake -B build -G "Visual Studio 17 2022"
