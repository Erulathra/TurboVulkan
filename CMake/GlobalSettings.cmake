set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/permissive)
    add_compile_options(/Zc:preprocessor)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DWITH_TURBO_64)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-DWITH_TURBO_32)
endif ()

#if (EXISTS "/usr/bin/mold" AND UNIX)
#    add_link_options("-fuse-ld=mold")
#endif ()

if (MSVC)
    add_definitions(-DWITH_MSVC)
endif ()

