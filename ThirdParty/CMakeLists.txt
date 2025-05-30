# CPM - package manager
include(CPM)

# Vulkan memory allocator
CPMAddPackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@3.3.0")
# Lib which enhances enums
CPMAddPackage("gh:Neargye/magic_enum@0.9.7")
# events and delegate support
CPMAddPackage( "gh:wqking/eventpp@0.1.3")
# Math library
CPMAddPackage(
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "1.0.1"
        OPTIONS "GLM_ENABLE_SIMD_AVX2 ON"
)

if (WIN32)
    set(VOLK_OPTIONS "VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR" )
else()
    set(VOLK_OPTIONS "VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR" )
endif ()

# Vulkan function auto loader
CPMAddPackage(
        GITHUB_REPOSITORY "zeux/volk"
        GIT_TAG "1.4.304"
        OPTIONS VOLK_OPTIONS
)
