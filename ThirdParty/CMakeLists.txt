# CPM - package manager
include(CPM)

CPMAddPackage("gh:KhronosGroup/Vulkan-Headers@1.4.313")

# SDL setup
CPMAddPackage(
        GITHUB_REPOSITORY "libsdl-org/SDL"
        GIT_TAG "release-3.2.16"
)

# Lib which enhances enums
CPMAddPackage("gh:Neargye/magic_enum@0.9.7")
# events and delegate support
CPMAddPackage( "gh:fmtlib/fmt#11.2.0")
CPMAddPackage( "gh:gabime/spdlog@1.15.3")
# Math library
CPMAddPackage(
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "1.0.1"
        OPTIONS "GLM_ENABLE_SIMD_AVX2 ON"
)

CPMAddPackage(
        GITHUB_REPOSITORY "charles-lunarg/vk-bootstrap"
        GIT_TAG "v1.4.313"
        OPTIONS "VK_BOOTSTRAP_POSITION_INDEPENDENT_CODE ON"
)

# Vulkan memory allocator
CPMAddPackage(
        GITHUB_REPOSITORY "GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator"
        GIT_TAG "v3.2.1"
        OPTIONS "VMA_BUILD_SAMPLES OFF"
)

CPMAddPackage( "gh:YaaZ/VulkanMemoryAllocator-Hpp@3.2.1")

CPMAddPackage("gh:ocornut/imgui@1.92.3-docking")
add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp

        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp

        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_link_libraries(imgui PRIVATE Vulkan-Headers)
target_compile_definitions(imgui PUBLIC -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

CPMAddPackage("gh:wolfpld/tracy@0.12.2")
CPMAddPackage("gh:spnda/fastgltf@0.9.0")

add_subdirectory(Slang)