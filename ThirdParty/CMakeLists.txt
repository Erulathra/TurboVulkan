# CPM - package manager
include(CPM)

add_subdirectory(STB)

# Lib which enhances enums
CPMAddPackage("gh:Neargye/magic_enum@0.9.7")

# Math library
CPMAddPackage(
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "8f6213d"
        OPTIONS "GLM_ENABLE_SIMD_AVX2 ON"
)

# Vulkan memory allocator
CPMAddPackage(
        GITHUB_REPOSITORY "GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator"
        GIT_TAG "v3.2.1"
        OPTIONS "VMA_BUILD_SAMPLES OFF"
)
CPMAddPackage( "gh:YaaZ/VulkanMemoryAllocator-Hpp@3.2.1")

find_package(VulkanHeaders CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

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
target_link_libraries(turbo_imgui PRIVATE Vulkan::Headers)
target_link_libraries(turbo_imgui PRIVATE SDL3::SDL3)

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

CPMAddPackage("gh:spnda/dds_image#6af8726")
CPMAddPackage("gh:dougbinks/enkiTS#0289cf6")

add_subdirectory(RenderDoc)
