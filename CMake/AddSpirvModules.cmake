#[[
BSD 3-Clause License

Copyright (c) 2022 Lili Hempel
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]

cmake_minimum_required(VERSION 3.19)

# Adds custom commands to compile a number of shader source files to
# SPIR-V using glslc and bundles them into a custom target.
function(add_spirv_modules TARGET_NAME)
    # Find glslc
    find_package(Vulkan REQUIRED glslc)

    # Parse arguments
    cmake_parse_arguments(PARSE_ARGV 1 "ARG"
            ""
            "SOURCE_DIR;BINARY_DIR"
            "SOURCES;OPTIONS"
    )

    # Adjust arguments / provide defaults
    if(NOT DEFINED ARG_SOURCE_DIR)
        set(ARG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    elseif(NOT IS_ABSOLUTE ${ARG_SOURCE_DIR})
        set(ARG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SOURCE_DIR})
    endif()

    if(NOT DEFINED ARG_BINARY_DIR)
        set(ARG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    elseif(NOT IS_ABSOLUTE ${ARG_BINARY_DIR})
        set(ARG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARG_BINARY_DIR})
    endif()

    # Define custom compilation commands
    foreach(FILE IN LISTS ARG_SOURCES)
        set(SOURCE_FILE ${ARG_SOURCE_DIR}/${FILE})
        set(BINARY_FILE ${ARG_BINARY_DIR}/${FILE}.spv)
        file(RELATIVE_PATH BIN_FILE_REL_PATH ${CMAKE_BINARY_DIR} ${BINARY_FILE})

        add_custom_command(
                OUTPUT          ${BINARY_FILE}
                COMMAND         ${Vulkan_GLSLC_EXECUTABLE}
                ${SOURCE_FILE}
                -o ${BINARY_FILE}
                ${ARG_OPTIONS}
                MAIN_DEPENDENCY ${SOURCE_FILE}
                COMMENT         "Building SPIR-V shader ${BIN_FILE_REL_PATH}"
                VERBATIM
                COMMAND_EXPAND_LISTS
        )

        list(APPEND BINARIES ${BINARY_FILE})
    endforeach()

    # Create target consisting of all compilation results
    add_custom_target(${TARGET_NAME} DEPENDS ${BINARIES})

endfunction()