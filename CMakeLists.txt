cmake_minimum_required(VERSION 3.12)
project(BlockGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Internal;Debug;Release")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GameBase)

add_custom_target(Utilities ALL SOURCES
    embed_shaders.bat
    run_clang_tidy.bat
    .clang-format
    .clang-tidy
    .gitignore
)
source_group("Script Files" FILES embed_shaders.bat run_clang_tidy.bat)
source_group("Config Files" FILES .clang-format .clang-tidy .gitignore)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    set(CMAKE_CXX_FLAGS_INTERNAL "${CMAKE_CXX_FLAGS_INTERNAL} /DDEBUG /O2")
    
    set(CMAKE_EXE_LINKER_FLAGS_INTERNAL "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    set(CMAKE_SHARED_LINKER_FLAGS_INTERNAL "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_INTERNAL "-O2 -DDEBUG")
endif()

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2-2.30.9")
set(SDL2_mixer_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_mixer-2.8.0")

list(APPEND CMAKE_PREFIX_PATH "${SDL2_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${SDL2_mixer_DIR}/cmake")

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

add_subdirectory(GameBase)
add_subdirectory(GameCode)
add_subdirectory(ShaderEmbedder)

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")