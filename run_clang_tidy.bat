@echo off
setlocal enabledelayedexpansion

:: Set include paths
set INCLUDES=-I./lib -I./GameEngine -I./GameCode -IC:/SDL2-2.0.9/include

:: Run clang-tidy
echo Running clang-tidy...
echo Include paths: %INCLUDES%

clang-tidy --fix --fix-errors GameCode/*.cpp GameCode/*.h GameEngine/*.cpp GameEngine/*.h ShaderEmbedder/*.cpp -- -std=c++17 %INCLUDES% -fms-compatibility -fdelayed-template-parsing
clang-format -i GameCode/*.cpp GameCode/*.h GameEngine/*.cpp GameEngine/*.h ShaderEmbedder/*.cpp

if %ERRORLEVEL% NEQ 0 (
    echo Error running clang-tidy
    exit /b 1
)

echo Done!
