############################################################################################
# C++ standard
############################################################################################

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

############################################################################################
# CMAKE LOG FUNCTION
############################################################################################

function(turbo_message VERBOSITY MESSAGE)
    message(${VERBOSITY} "[TURBO] ${MESSAGE}")
endfunction()

############################################################################################
# Build configuration
############################################################################################

set(BUILD_TYPE false CACHE STRING "Build type")

string(COMPARE EQUAL "${BUILD_TYPE}" "Development" TURBO_BUILD_DEVELOPMENT)
string(COMPARE EQUAL "${BUILD_TYPE}" "Shipping" TURBO_BUILD_SHIPPING)

# In case when BUILD_TYPE is missing set build type to development
if (NOT ${TURBO_BUILD_SHIPPING} AND NOT ${TURBO_BUILD_DEVELOPMENT})
    turbo_message(STATUS "BUILD_TYPE argument is missing. Setting build type to DEVELOPMENT")
    set(TURBO_BUILD_DEVELOPMENT 1)
else ()
    turbo_message(STATUS "Set BUILD_TYPE to ${BUILD_TYPE}")
endif()

SET(WITH_PROFILER ${TURBO_BUILD_DEVELOPMENT})
turbo_message(STATUS "Set WITH_PROFILER to ${WITH_PROFILER}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
    turbo_message(STATUS "Enable debug build type.")
endif ()

############################################################################################
# General
############################################################################################

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    turbo_message(FATAL_ERROR "Project ${PROJECT_NAME} supports only 64bit architectures.")
endif ()

if (EXISTS "/usr/bin/mold" AND UNIX)
    add_link_options("-fuse-ld=mold")
    turbo_message(STATUS "Enable mold linker.")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

############################################################################################
# MSVC
############################################################################################

if (MSVC)
    # Make MSVC more standard compliant.
    add_compile_options(/permissive)
    add_compile_options(/Zc:preprocessor)

    turbo_message(STATUS "Platform MSVC")
endif ()

add_compile_definitions(PLATFORM_MSVC=${MSVC})

############################################################################################
# Linux
############################################################################################

if (UNIX)
    turbo_message(STATUS "Platform Linux")
endif ()

add_compile_definitions(PLATFORM_LINUX=${UNIX})

############################################################################################
# Other
############################################################################################

if (NOT UNIX AND NOT MSVC)
    turbo_message(FATAL "Unsupported target system.")
endif ()
