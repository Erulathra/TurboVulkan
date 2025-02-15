project(TurboVulcan_Engine VERSION 0.0.1)

# Add private files
file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)

# Add public files
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

add_library(${PROJECT_NAME} ${PUBLIC_FILES} ${PRIVATE_FILES})

# Set target language
set_target_properties(TurboVulcan_Engine PROPERTIES LINKER_LANGUAGE CXX)

target_precompile_headers(${PROJECT_NAME} PUBLIC pch.h)

target_include_directories(${PROJECT_NAME} PUBLIC Public)

target_link_libraries(${PROJECT_NAME} INTERFACE stdc++)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE spdlog::spdlog spdlog::spdlog_header_only)

find_package(SDL3 REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE SDL3)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE glm::glm)

find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})

add_dependencies(${PROJECT_NAME} TurboVulcan_Shaders)



