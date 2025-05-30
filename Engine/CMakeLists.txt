project(TurboVulcan_Engine VERSION 0.1.0)

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

target_link_libraries(${PROJECT_NAME} PUBLIC eventpp::eventpp)
target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum)

find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

find_package(SDL3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3)

find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glm)

target_link_libraries(${PROJECT_NAME} PRIVATE volk)
add_compile_definitions(VK_NO_PROTOTYPES)

find_package(VulkanMemoryAllocator REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

add_dependencies(${PROJECT_NAME} TurboVulcan_Shaders)

add_compile_definitions(TURBO_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(TURBO_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_compile_definitions(TURBO_VERSION_PATCH=${PROJECT_VERSION_PATCH})




