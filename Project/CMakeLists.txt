project(TurboVulkan_Sample VERSION 0.1.0)

# Add private files
file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)

# Add public files
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

include_directories(Public)

add_executable(${PROJECT_NAME} ${PUBLIC_FILES} ${PRIVATE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_link_libraries(${PROJECT_NAME} TurboVulkan)

target_copy_slang_binaries(${PROJECT_NAME})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Content
        ${CMAKE_CURRENT_BINARY_DIR}/Content
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Content
        ${CMAKE_CURRENT_BINARY_DIR}/Content
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Saved
        ${CMAKE_CURRENT_BINARY_DIR}/Saved
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/Shaders
        ${CMAKE_CURRENT_BINARY_DIR}/Shaders
)

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )
endif ()
