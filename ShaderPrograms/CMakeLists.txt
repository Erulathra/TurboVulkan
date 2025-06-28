cmake_minimum_required(VERSION 3.20)

function(add_slang_modules TARGET_NAME)
    set(SLANG_EXECUTABLE $ENV{SLANG_PATH}/slangc)

    if (WIN32)
        set(SLANG_EXECUTABLE ${SLANG_EXECUTABLE}.exe)
    endif ()

    # Parse arguments
    cmake_parse_arguments(PARSE_ARGV 1 "ARG"
            ""
            "SOURCE_DIR"
            "SOURCES"
    )

    # Adjust arguments / provide defaults
    if(NOT DEFINED ARG_SOURCE_DIR)
        set(ARG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    elseif(NOT IS_ABSOLUTE ${ARG_SOURCE_DIR})
        set(ARG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SOURCE_DIR})
    endif()

    set(ARG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

    # Define custom compilation commands
    foreach(FILE IN LISTS ARG_SOURCES)
        string(REPLACE ".slang" ".spv" BINARY_FILE_NAME ${FILE})

        set(SOURCE_FILE ${ARG_SOURCE_DIR}/${FILE})
        set(BINARY_FILE ${ARG_BINARY_DIR}/${BINARY_FILE_NAME})
        file(RELATIVE_PATH BIN_FILE_REL_PATH ${CMAKE_BINARY_DIR} ${BINARY_FILE})

        add_custom_command(
                OUTPUT          ${BINARY_FILE}
                COMMAND         ${SLANG_EXECUTABLE}
                ${SOURCE_FILE}
                -profile glsl_450
                -target spirv
                -o ${BINARY_FILE}
                -I ${PROJECT_SOURCE_DIR}/ShaderModules
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

file(GLOB_RECURSE SHADER_SOURCE_FILES
        *.slang)

list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)

if(FILE_COUNT GREATER 0)

    set(SHADER_FILES)

    foreach(SHADER_PATH IN LISTS SHADER_SOURCE_FILES)
        file(RELATIVE_PATH SHADER_RELATIVE_PATH ${PROJECT_SOURCE_DIR} ${SHADER_PATH})
        list(APPEND SHADER_FILES ${SHADER_RELATIVE_PATH})
    endforeach()

    # Create shader target
    add_slang_modules(TurboVulcan_Shaders
            SOURCE_DIR ${PROJECT_SOURCE_DIR}
            SOURCES    ${SHADER_FILES})
endif()