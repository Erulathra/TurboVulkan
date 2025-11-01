project(TurboVulkan VERSION 0.1.0)

############################################################################################
# Project configuration
############################################################################################

file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

add_library(${PROJECT_NAME} STATIC ${PUBLIC_FILES} ${PRIVATE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_precompile_headers(${PROJECT_NAME} PUBLIC pch.h)

target_include_directories(${PROJECT_NAME} PRIVATE Private)
target_include_directories(${PROJECT_NAME} PUBLIC Public)

############################################################################################
# Libraries
############################################################################################

if (UNIX)
    target_link_libraries(${PROJECT_NAME} INTERFACE stdc++)
    target_link_libraries(${PROJECT_NAME} PUBLIC -lm)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan-Headers)
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3-shared)
target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC fastgltf::fastgltf)

# TODO: Create GPU Interface and make those private
target_link_libraries(${PROJECT_NAME} PUBLIC GPUOpen::VulkanMemoryAllocator)
target_link_libraries(${PROJECT_NAME} PUBLIC VulkanMemoryAllocator-Hpp)
target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
target_link_libraries(${PROJECT_NAME} PUBLIC EnTT::EnTT)

target_link_libraries(${PROJECT_NAME} PRIVATE slang)

############################################################################################
# Tracy configuration
############################################################################################

target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_PROFILER=${WITH_PROFILER})

if (${WITH_PROFILER})
    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)

    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_ENABLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_ON_DEMAND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_VK_USE_SYMBOL_TABLE)
endif ()

############################################################################################
# Project defines
############################################################################################

target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_VERSION_PATCH=${PROJECT_VERSION_PATCH})

target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_BUILD_DEVELOPMENT=${TURBO_BUILD_DEVELOPMENT})
target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_BUILD_SHIPPING=${TURBO_BUILD_SHIPPING})

target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_ASSERTIONS=${TURBO_BUILD_DEVELOPMENT})
target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_VALIDATION_LAYERS=${TURBO_BUILD_DEVELOPMENT})
target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_DEBUG_RENDERING_FEATURES=${TURBO_BUILD_DEVELOPMENT})
