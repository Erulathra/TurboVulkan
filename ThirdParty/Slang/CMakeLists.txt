project(slang VERSION 2025.16)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/Slang")

add_library(libgfx SHARED IMPORTED)
add_library(libslang SHARED IMPORTED)
add_library(libslang-glslang SHARED IMPORTED)
add_library(libslang-glsl-module SHARED IMPORTED)
#add_library(libslang-llvm SHARED IMPORTED)
add_library(libslang-rt SHARED IMPORTED)
if (WIN32)
    # TODO:
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_ROOT}/Include
#            IMPORTED_IMPLIB ${PROJECT_ROOT}/Lib/Windows/slang.lib
#            IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Windows/slang.dll
#    )
else ()
    set_target_properties(libgfx PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libgfx.so)
    set_target_properties(libslang PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang.so)
    set_target_properties(libslang-glslang PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-glslang.so)
    set_target_properties(libslang-glsl-module PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-glsl-module.so)
#    set_target_properties(libslang-llvm PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-llvm.so)
    set_target_properties(libslang-rt PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-rt.so)
endif ()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_ROOT}/Include)
target_link_libraries(${PROJECT_NAME} INTERFACE libslang)
