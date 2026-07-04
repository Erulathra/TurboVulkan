# CPM - package manager
include(CPM)

add_subdirectory(STB)

CPMAddPackage("gh:KhronosGroup/Vulkan-Headers@1.4.313")

# SDL setup
CPMAddPackage(
        GITHUB_REPOSITORY "libsdl-org/SDL"
        GIT_TAG "release-3.4.10"
)

# Lib which enhances enums
CPMAddPackage("gh:Neargye/magic_enum@0.9.7")
# events and delegate support
CPMAddPackage( "gh:fmtlib/fmt#12.2.0")
CPMAddPackage( "gh:gabime/spdlog@1.15.3")
# Math library
CPMAddPackage(
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "8f6213d"
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
add_library(turbo_imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp

        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp

        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_link_libraries(turbo_imgui PRIVATE Vulkan-Headers)
target_link_libraries(turbo_imgui PRIVATE SDL3::SDL3-shared)
target_compile_definitions(turbo_imgui PUBLIC -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES)
target_compile_definitions(turbo_imgui PUBLIC -DIMGUI_USE_WCHAR32)
target_link_libraries(turbo_imgui PRIVATE glm::glm)

target_include_directories(turbo_imgui PUBLIC ${imgui_SOURCE_DIR})
target_include_directories(turbo_imgui PUBLIC ImGui)
target_compile_definitions(turbo_imgui PUBLIC -DIMGUI_USER_CONFIG="ImGuiConfig.h")

CPMAddPackage("gh:CedricGuillemet/ImGuizmo#be8aa4a")
add_library(turbo_imguizmo STATIC
        ${ImGuizmo_SOURCE_DIR}/src/ImGuizmo.cpp
        ${ImGuizmo_SOURCE_DIR}/src/GraphEditor.cpp
        ${ImGuizmo_SOURCE_DIR}/src/ImCurveEdit.cpp
        ${ImGuizmo_SOURCE_DIR}/src/ImGradient.cpp
        ${ImGuizmo_SOURCE_DIR}/src/ImSequencer.cpp
        ${ImGuizmo_SOURCE_DIR}/src/ImVectorEditor.cpp
)
target_link_libraries(turbo_imguizmo PRIVATE turbo_imgui)
target_link_libraries(turbo_imguizmo PRIVATE glm::glm)
target_include_directories(turbo_imguizmo PUBLIC ${ImGuizmo_SOURCE_DIR}/src)

CPMAddPackage(
        GITHUB_REPOSITORY "wolfpld/tracy"
        GIT_TAG "v0.12.2"
        OPTIONS "TRACY_ON_DEMAND ON"
)

CPMAddPackage("gh:spnda/fastgltf@0.9.0")
CPMAddPackage("gh:skypjack/entt@3.15.0")

CPMAddPackage(
        GITHUB_REPOSITORY "skypjack/entt"
        GIT_TAG "v3.15.0"
        OPTIONS "ENTT_INCLUDE_NATVIS ON"
)
CPMAddPackage("gh:spnda/dds_image#6af8726")
CPMAddPackage("gh:dougbinks/enkiTS#0289cf6")

CPMAddPackage(
        GITHUB_REPOSITORY "shader-slang/slang"
        GIT_TAG "v2026.12"
        OPTIONS "SLANG_ENABLE_DXIL OFF"
                "SLANG_ENABLE_GFX OFF"
                "SLANG_ENABLE_SLANGD OFF"
                "SLANG_ENABLE_SLANGC OFF"
                "SLANG_ENABLE_SLANGI OFF"
                "SLANG_ENABLE_TESTS OFF"
                "SLANG_ENABLE_EXAMPLES OFF"
                "SLANG_ENABLE_REPLAYER OFF"
                "SLANG_ENABLE_SLANG_PROXY OFF"
                "SLANG_ENABLE_CUDA OFF"
                "SLANG_ENABLE_OPTIX OFF"
                "SLANG_ENABLE_NVAPI OFF"
                "SLANG_ENABLE_AFTERMATH OFF"
                "SLANG_ENABLE_XLIB OFF"
)

add_subdirectory(RenderDoc)
