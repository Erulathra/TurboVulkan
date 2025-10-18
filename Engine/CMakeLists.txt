project(TurboVulcan_Engine VERSION 0.1.0)

# Add private files
file(GLOB_RECURSE PRIVATE_FILES Private/*.cpp)

# Add public files
file(GLOB_RECURSE PUBLIC_FILES Public/*.h)

add_library(${PROJECT_NAME} STATIC ${PUBLIC_FILES} ${PRIVATE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

# Set target language
set_target_properties(TurboVulcan_Engine PROPERTIES LINKER_LANGUAGE CXX)

target_precompile_headers(${PROJECT_NAME} PUBLIC pch.h)

target_include_directories(${PROJECT_NAME} PRIVATE Private)
target_include_directories(${PROJECT_NAME} PUBLIC Public)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} INTERFACE stdc++)
    target_link_libraries(${PROJECT_NAME} PUBLIC -lm)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan-Headers)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3-shared)

target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC fastgltf::fastgltf)

# TODO: Create GPU Interface and make those private
target_link_libraries(${PROJECT_NAME} PUBLIC GPUOpen::VulkanMemoryAllocator)
target_link_libraries(${PROJECT_NAME} PUBLIC VulkanMemoryAllocator-Hpp)
target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)

target_link_libraries(${PROJECT_NAME} PRIVATE slang)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

set(BuildType false CACHE STRING "Build type")

set(BuildType_Development "Development")
set(BuildType_Shipping "Shipping")

if (${BuildType} STREQUAL BuildType_Shipping)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_BUILD_DEVELOPMENT=0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_BUILD_SHIPPING=1)
    set(TURBO_BUILD_SHIPPING true)
else ()
    target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_BUILD_DEVELOPMENT=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_BUILD_SHIPPING=0)
    set(TURBO_BUILD_DEVELOPMENT true)
endif ()

target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(${PROJECT_NAME} PUBLIC TURBO_VERSION_PATCH=${PROJECT_VERSION_PATCH})

# Vulkan HPP definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_NO_EXCEPTIONS)
target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

# GLM definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

if (${TURBO_BUILD_SHIPPING})
    SET(WITH_PROFILER false)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_ASSERTIONS=0)
    target_compile_definitions(&{PROJECT_NAME} PUBLIC WITH_VALIDATION_LAYERS=0)
    target_compile_definitions(&{PROJECT_NAME} PUBLIC WITH_DEBUG_RENDERING_FEATURES=0)
else ()
    SET(WITH_PROFILER true)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_ASSERTIONS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_VALIDATION_LAYERS=1)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_DEBUG_RENDERING_FEATURES=1)
endif ()


# Tracy profiler support
if (${WITH_PROFILER})
    message("[TURBO] Enabling profiling support")

    target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)

    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_ENABLE)
#    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_ON_DEMAND)
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_VK_USE_SYMBOL_TABLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_PROFILER=1)

else ()
    message("[TURBO] Disabling profiling support")

    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_PROFILER=0)
endif ()




