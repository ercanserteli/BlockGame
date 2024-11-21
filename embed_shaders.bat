@echo off
setlocal enabledelayedexpansion

:: Use the first argument as the configuration
set CONFIG=%~1

set INPUT_DIR=GameCode
set OUTPUT_DIR=GameCode\shaders
set FILE_FOUND=0

if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

for %%F in (%INPUT_DIR%\*.glsl) do (
    set FILE_NAME=%%~nF
    set SOURCE_TYPE=!FILE_NAME!_source
    set OUTPUT_FILE=!OUTPUT_DIR!\!FILE_NAME!_glsl.h

    build\ShaderEmbedder\%CONFIG%\ShaderEmbedder.exe !SOURCE_TYPE! "%%F" "!OUTPUT_FILE!"

    set /a FILE_FOUND+=1
)

if %FILE_FOUND% EQU 0 (
    echo No shaders were found in the directory "%INPUT_DIR%".
) else (
    echo All shaders have been embedded.
)
