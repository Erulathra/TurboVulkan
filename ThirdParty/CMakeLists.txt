# CPM - package manager
include(CPM)

# Vulkan memory allocator
CPMAddPackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@3.3.0")
# Lib which enhances enums
CPMAddPackage("gh:Neargye/magic_enum@0.9.7")
# events and delegate support
CPMAddPackage( "gh:wqking/eventpp@0.1.3")
CPMAddPackage( "gh:fmtlib/fmt#11.2.0")
CPMAddPackage( "gh:gabime/spdlog@1.15.3")
# Math library
CPMAddPackage(
        GITHUB_REPOSITORY "g-truc/glm"
        GIT_TAG "1.0.1"
        OPTIONS "GLM_ENABLE_SIMD_AVX2 ON"
)
