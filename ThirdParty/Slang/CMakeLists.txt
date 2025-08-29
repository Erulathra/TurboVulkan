project(slang VERSION 2025.15.1)

add_library(${PROJECT_NAME} SHARED IMPORTED GLOBAL)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/Slang")

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_ROOT}/Include
            IMPORTED_IMPLIB ${PROJECT_ROOT}/Lib/Windows/slang.lib
            IMPORTED_LOCATION ${PROJECT_ROOT}/Bin/Windows/slang.dll
    )
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_ROOT}/Include
            IMPORTED_LOCATION ${PROJECT_ROOT}/Lib/Linux/libslang.so
    )
endif ()
