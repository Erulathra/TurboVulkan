# This file was part of the "Slang x WebGPU" demo, but it is modified by Turbo
# Vulkan authors to meet their requirements.
#   https://github.com/eliemichel/SlangWebGPU
#
# MIT License
# Copyright (c) 2024 Elie Michel and
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

include(FetchContent)

############################################################################################
# Options
############################################################################################

set(SLANG_VERSION "2025.19.1" CACHE STRING "Version of the Slang release to use, without the leading 'v'. Must correspond to a release available at https://github.com/shader-slang/slang/releases (or whatever the SLANG_MIRROR variable is set to).")
set(SLANG_MIRROR "https://github.com/shader-slang/slang" CACHE STRING "This is the source from which release binaries are fetched, set by default to official Slang repository but you may change it to use a fork.")

############################################################################################
# Build URL to fetch
############################################################################################

set(URL_OS)
if (MSVC)
	set(URL_OS "windows")
elseif (UNIX)
	set(URL_OS "linux")
else()
	turbo_message(FATAL "Target system is not supported")
endif()

set(URL_NAME "slang-${SLANG_VERSION}-${URL_OS}-x86_64")
string(TOLOWER "${URL_NAME}" FC_NAME)
set(URL "${SLANG_MIRROR}/releases/download/v${SLANG_VERSION}/${URL_NAME}.zip")

############################################################################################
# Declare FetchContent, then make available
############################################################################################

FetchContent_Declare(${FC_NAME} URL ${URL})
turbo_message(STATUS "Using Slang binaries from '${URL}'")
FetchContent_MakeAvailable(${FC_NAME})
set(Slang_ROOT "${${FC_NAME}_SOURCE_DIR}" CACHE INTERNAL "Root directory of fetched Slang binaries")

############################################################################################
# Import targets (ideally slang would provide a SlangConfig.cmake)
############################################################################################

add_library(slang SHARED IMPORTED GLOBAL)

if (MSVC)
	set_target_properties(
		slang
		PROPERTIES
			IMPORTED_IMPLIB "${Slang_ROOT}/lib/slang.lib"
			IMPORTED_LOCATION "${Slang_ROOT}/bin/slang.dll"
	)
elseif (UNIX)
	set_target_properties(
			slang
			PROPERTIES
			IMPORTED_LOCATION "${Slang_ROOT}/lib/libslang.so"
			IMPORTED_NO_SONAME TRUE
	)
else()
	message(FATAL_ERROR "Sorry, Slang does not provide precompiled binaries for MSYS/MinGW")
endif()

target_include_directories(slang INTERFACE
	"${Slang_ROOT}/include"
)

############################################################################################
# Utility function
############################################################################################

function(target_copy_slang_binaries TargetName)
	if (MSVC)
		file(GLOB_RECURSE SLANG_BINARIES "${Slang_ROOT}/bin/*.dll")
	elseif (UNIX)
		file(GLOB_RECURSE SLANG_BINARIES "${Slang_ROOT}/bin/*.so")
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
