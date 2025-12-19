project(TurboVulkan_Editor VERSION 0.1.0)

# Add private files
file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)

# Add public files
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

include_directories(Public)

add_executable(${PROJECT_NAME} ${PUBLIC_FILES} ${PRIVATE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_link_libraries(${PROJECT_NAME} TurboVulkan)

setup_project_directory(${PROJECT_NAME})