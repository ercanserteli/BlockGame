project(GameBase)

add_executable(GameBase GameBase.cpp)

file(GLOB HEADER_FILES "*.h")
target_sources(GameBase PRIVATE ${HEADER_FILES})
source_group("Header Files" FILES ${HEADER_FILES})

include_directories(${SDL2_DIR}/../include)
include_directories(${SDL2_mixer_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/GameCode)

target_link_libraries(GameBase PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_mixer::SDL2_mixer)

add_dependencies(GameBase GameCode)

set_target_properties(GameBase PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
    RUNTIME_OUTPUT_DIRECTORY_INTERNAL ${CMAKE_BINARY_DIR}/Internal
)

set(SDL_DLLS
    "${SDL2_DIR}/../lib/x64/SDL2.dll"
    "${SDL2_mixer_DIR}/../lib/x64/SDL2_mixer.dll"
)
add_custom_command(TARGET GameBase POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL_DLLS}
        $<TARGET_FILE_DIR:GameBase>
)
add_custom_command(TARGET GameBase POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:GameBase>/assets
)