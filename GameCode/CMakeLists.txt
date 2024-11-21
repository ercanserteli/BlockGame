project(GameCode)

add_library(GameCode SHARED
    Game.cpp
    Graphics.cpp
    Play.cpp
    Sound.cpp
    Shader.cpp
    Chunk.cpp
    Frustum.cpp
    AABB.cpp
    Save.cpp
    Collision.cpp
    ShadowDebugVisuals.cpp
    ../lib/glad/glad.c
    ../lib/noise/SimplexNoise.cpp
)

file(GLOB HEADER_FILES "*.h")
file(GLOB SHADER_FILES "*.glsl")
set(SHADER_HEADER_FILES)
foreach(SHADER_FILE IN LISTS SHADER_FILES)
    get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME_WE)
    get_filename_component(SHADER_DIR "${SHADER_FILE}" DIRECTORY)
    set(SHADER_HEADER_FILE "${SHADER_DIR}/shaders/${SHADER_NAME}_glsl.h")
    list(APPEND SHADER_HEADER_FILES "${SHADER_HEADER_FILE}")
endforeach()

target_sources(GameCode PRIVATE ${HEADER_FILES} ${SHADER_FILES} ${SHADER_HEADER_FILES})
source_group("Header Files" FILES ${HEADER_FILES})
source_group("Shader Files" FILES ${SHADER_FILES})
source_group("Generated Files" FILES ${SHADER_HEADER_FILES})

include_directories(${SDL2_DIR}/../include)
include_directories(${SDL2_mixer_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/GameBase)

target_link_libraries(GameCode PRIVATE SDL2::SDL2 SDL2_mixer::SDL2_mixer)

add_custom_command(
    PRE_BUILD
    OUTPUT ${SHADER_HEADER_FILES}
    COMMAND cmd /c "${CMAKE_SOURCE_DIR}/embed_shaders.bat" $<CONFIG>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${SHADER_FILES}
    COMMENT "Embedding shaders in header files"
)

add_dependencies(GameCode ShaderEmbedder)

set_target_properties(GameCode PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
    RUNTIME_OUTPUT_DIRECTORY_INTERNAL ${CMAKE_BINARY_DIR}/Internal
)

