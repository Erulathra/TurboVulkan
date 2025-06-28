# Add private files
file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)

# Add public files
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

include_directories(Public)

add_executable(${PROJECT_NAME} ${PUBLIC_FILES} ${PRIVATE_FILES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_Engine)

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
        ${CMAKE_SOURCE_DIR}/Runtime
        ${CMAKE_CURRENT_BINARY_DIR}/Runtime
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/ShaderPrograms/ShaderPrograms
        ${CMAKE_CURRENT_BINARY_DIR}/Shaders
)
