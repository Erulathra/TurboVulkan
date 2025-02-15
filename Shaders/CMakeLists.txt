file(GLOB_RECURSE SHADER_SOURCE_FILES
        *.vert
        *.frag)

list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)

if(FILE_COUNT GREATER 0)

    set(SHADER_FILES)

    foreach(SHADER_PATH IN LISTS SHADER_SOURCE_FILES)
        file(RELATIVE_PATH SHADER_RELATIVE_PATH ${PROJECT_SOURCE_DIR} ${SHADER_PATH})
        list(APPEND SHADER_FILES ${SHADER_RELATIVE_PATH})
    endforeach()

    # Include required modules
    include(AddSpirvModules)

    # Create shader target
    add_spirv_modules(TurboVulcan_Shaders
            SOURCE_DIR ${PROJECT_SOURCE_DIR}
            BINARY_DIR
            SOURCES    ${SHADER_FILES})
endif()