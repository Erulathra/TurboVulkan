# CPM - package manager
include(CPM)

CPMAddPackage("gh:wqking/eventpp@0.1.3")
CPMAddPackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@3.3.0")

if (WIN32)
    set(VOLK_OPTIONS "VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR" )
else()
    set(VOLK_OPTIONS "VOLK_STATIC_DEFINES VK_USE_PLATFORM_WAYLAND_KHR" )
endif ()

CPMAddPackage(
        GITHUB_REPOSITORY "zeux/volk"
        GIT_TAG "1.4.304"
        OPTIONS VOLK_OPTIONS
)
#message("VOLK_OPTIONS: ${VOLK_OPTIONS}")
