project(slang VERSION 2025.16)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/Slang")

#add_library(libgfx MODULE IMPORTED)
add_library(slang SHARED IMPORTED GLOBAL)
#add_library(libslang-glslang MODULE IMPORTED)
#add_library(libslang-glsl-module MODULE IMPORTED)
#add_library(libslang-rt MODULE IMPORTED)

if (WIN32)
#    set_target_properties(
#            libgfx PROPERTIES
#            IMPORTED_IMPLIB ${PROJECT_ROOT}/Lib/Windows/gfx.lib
#            IMPORTED_LOCATION ${PROJECT_ROOT}/Bin/Windows/gfx.dll
#    )

    set_target_properties(
            slang PROPERTIES
            IMPORTED_IMPLIB ${PROJECT_ROOT}/Lib/Windows/slang.lib
            IMPORTED_LOCATION ${PROJECT_ROOT}/Bin/Windows/slang.dll
    )

#    set_target_properties(
#            libslang-glslang PROPERTIES
#            IMPORTED_LOCATION ${PROJECT_ROOT}/Bin/Windows/slang-glslang.dll
#    )
#
#    set_target_properties(
#            libslang-glsl-module PROPERTIES
#            IMPORTED_LOCATION ${PROJECT_ROOT}/Bin/Windows/slang-glsl-module.dll
#    )
#
#    set_target_properties(
#            libslang-rt PROPERTIES
#            IMPORTED_IMPLIB ${PROJECT_ROOT}/Lib/Windows/slang-rt.lib
#            IMPORTED_LOCATION ${PROJECT_ROOT}/Bin/Windows/slang-rt.dll
#    )
else ()
    set_target_properties(libgfx PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libgfx.so)
    set_target_properties(libslang PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang.so)
    set_target_properties(libslang-glslang PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-glslang.so)
    set_target_properties(libslang-glsl-module PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-glsl-module.so)
    set_target_properties(libslang-rt PROPERTIES IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang-rt.so)
endif ()

#add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(slang INTERFACE ${PROJECT_ROOT}/Include)
#target_link_libraries(${PROJECT_NAME} INTERFACE slang)