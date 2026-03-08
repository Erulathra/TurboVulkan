project(slang VERSION 2026.4)

set(SLANG_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/Slang" CACHE STRING "Root directory of Slang binaries")
add_library(slang SHARED IMPORTED GLOBAL)

if (NOT MSVC AND NOT UNIX)
	turbo_message(FATAL_ERROR "Unsupported platform" )
endif ()

if (MSVC)
	set_target_properties(
			slang
			PROPERTIES
			IMPORTED_IMPLIB "${SLANG_ROOT}/lib/slang.lib"
			IMPORTED_LOCATION "${SLANG_ROOT}/bin/slang.dll"
	)
elseif (UNIX)
	set_target_properties(
			slang
			PROPERTIES
			IMPORTED_LOCATION "${SLANG_ROOT}/lib/libslang.so"
			IMPORTED_NO_SONAME TRUE
	)
endif ()

target_include_directories(slang INTERFACE ${SLANG_ROOT}/include)
