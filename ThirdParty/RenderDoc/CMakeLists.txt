project(RenderDoc VERSION 1.42)

add_library(RenderDoc INTERFACE)
target_include_directories(RenderDoc INTERFACE ${CMAKE_SOURCE_DIR}/ThirdParty/RenderDoc/Include)
