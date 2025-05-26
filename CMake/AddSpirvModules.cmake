cmake_minimum_required(VERSION 3.19)

function(add_spirv_modules TARGET_NAME)
    set(SLANG_EXECUTABLE $ENV{SLANG_PATH}/slangc)

    if (WIN32)
        set(SLANG_EXECUTABLE ${SLANG_EXECUTABLE}.exe)
    endif ()

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