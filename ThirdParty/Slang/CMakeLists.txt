project(slang VERSION 2025.16)

set(SLANG_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/Slang" CACHE STRING "Root directory of Slang binaries")
add_library(slang SHARED IMPORTED GLOBAL)

if (NOT MSVC AND NOT UNIX)
	turbo_message(FATAL_ERROR "Unsupported platform" )
endif ()

if (MSVC)
	set_target_properties(
			slang
			PROPERTIES
			IMPORTED_IMPLIB "${SLANG_ROOT}/Windows/lib/slang.lib"
			IMPORTED_LOCATION "${SLANG_ROOT}/Windows/bin/slang.dll"
	)
elseif (UNIX)
	set_target_properties(
			slang
			PROPERTIES
			IMPORTED_LOCATION "${SLANG_ROOT}/Linux/lib/libslang.so"
			IMPORTED_NO_SONAME TRUE
	)
endif ()

target_include_directories(slang INTERFACE ${SLANG_ROOT}/Include)

function(target_copy_slang_binaries TargetName)
	turbo_message(STATUS "Coping slang binaries to target ${TargetName} directory.")

	if (MSVC)
		file(GLOB SLANG_BINARIES "${SLANG_ROOT}/Windows/bin/*.dll")
	elseif (UNIX)
		file(GLOB SLANG_BINARIES "${SLANG_ROOT}/Linux/lib/*.so")
	endif ()

	foreach (SLANG_BINARY ${SLANG_BINARIES})
		add_custom_command(
				TARGET ${TargetName} POST_BUILD
				COMMAND
				${CMAKE_COMMAND} -E copy_if_different
				${SLANG_BINARY}
				$<TARGET_FILE_DIR:${TargetName}>
				COMMENT
				"Copying '${SLANG_BINARY}' to '$<TARGET_FILE_DIR:${TargetName}>'..."
		)
	endforeach ()
endfunction(target_copy_slang_binaries)
