cmake_minimum_required(VERSION 3.30 FATAL_ERROR)
project(TurboVulkan VERSION 0.0.1)

message("Build type: ${CMAKE_BUILD_TYPE}")

# Include CMake directory as module
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Include Global settings
include(GlobalSettings)

# Third Party libs
add_subdirectory(ThirdParty)

# Engine Project
add_subdirectory(Engine)

# Game Project
add_subdirectory(Project)

# Editor Project
add_subdirectory(Editor)
