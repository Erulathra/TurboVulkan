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
# Project setup helper functions
############################################################################################

function(disable_rtti TARGET)
    target_compile_options(${TARGET} PRIVATE -fno-rtti)
endfunction()

function(disable_exceptions TARGET)
    target_compile_options(${TARGET} PRIVATE -fno-exceptions)
endfunction()

function(setup_project_directory TARGET)
    add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/Content
            ${CMAKE_CURRENT_BINARY_DIR}/Content
    )

    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/Saved)
    add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/Saved
            ${CMAKE_CURRENT_BINARY_DIR}/Saved
    )

    add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/Shaders
            ${CMAKE_CURRENT_BINARY_DIR}/Shaders
    )

    if (WIN32)
        add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
                COMMAND_EXPAND_LISTS
        )
    endif ()

    setup_slang_binaries(${TARGET})

endfunction()

############################################################################################
# Build configuration
############################################################################################

set(BUILD_TYPE false CACHE STRING "Build type")

string(COMPARE EQUAL "${BUILD_TYPE}" "Development" TURBO_BUILD_DEVELOPMENT)
string(COMPARE EQUAL "${BUILD_TYPE}" "Test" TURBO_BUILD_TEST)
string(COMPARE EQUAL "${BUILD_TYPE}" "Shipping" TURBO_BUILD_SHIPPING)

# In case when BUILD_TYPE is missing set build type to development
if (NOT ${TURBO_BUILD_SHIPPING} AND NOT ${TURBO_BUILD_DEVELOPMENT} AND NOT ${TURBO_BUILD_TEST})
    turbo_message(STATUS "BUILD_TYPE argument is missing. Setting build type to DEVELOPMENT")
    set(TURBO_BUILD_DEVELOPMENT 1)
else ()
    turbo_message(STATUS "Set BUILD_TYPE to ${BUILD_TYPE}")
endif()

if (NOT ${TURBO_BUILD_SHIPPING})
    SET(WITH_PROFILER 1)
else ()
    SET(WITH_PROFILER 0)
endif ()

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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

############################################################################################
# OS
############################################################################################

if (UNIX)
    turbo_message(STATUS "Platform Linux")
    set(WIN32 0)
elseif (WIN32)
    turbo_message(STATUS "Platform Windows")
    set(UNIX 0)
endif ()

add_compile_definitions(PLATFORM_LINUX=${UNIX})
add_compile_definitions(PLATFORM_WINDOWS=${WIN32})

############################################################################################
# Other
############################################################################################

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    turbo_message(FATAL_ERROR "Project only support CLANG compiler.")
endif ()

# Enable colored output
set(CMAKE_COLOR_DIAGNOSTICS ON)
