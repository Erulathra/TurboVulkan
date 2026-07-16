set(SLANG_VERSION "2026.12")

set(SLANG_ARCHIVE_LINUX slang-${SLANG_VERSION}-linux-x86_64.tar.gz)
set(SLANG_ARCHIVE_WIN slang-${SLANG_VERSION}-windows-x86_64.zip)

# Set archive name based on platform
set(SLANG_ARCHIVE ${SLANG_ARCHIVE_LINUX})
set(SLANG_TEST_PATH ${SLANG_ROOT}/lib/libslang-compiler.so.0.${SLANG_VERSION})
if (WIN32)
   set(SLANG_ARCHIVE ${SLANG_ARCHIVE_WIN})
   set(SLANG_TEST_PATH ${SLANG_ROOT}/bin/slang-compiler.dll)
endif()
set(SLANG_URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/${SLANG_ARCHIVE})

set(SLANG_ROOT ${CMAKE_BINARY_DIR}/_deps/slang)


# Download slang if not already present
if (NOT EXISTS ${SLANG_ROOT}/${SLANG_ARCHIVE})
   file(MAKE_DIRECTORY "${SLANG_INSTALL_DIR}")

   turbo_message(STATUS "Downloading slang ${SLANG_URL}...")
   file (
      DOWNLOAD
      ${SLANG_URL}
      ${SLANG_ROOT}/${SLANG_ARCHIVE}
      SHOW_PROGRESS
      STATUS DOWNLOAD_STATUS
   )

   list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)

   if (NOT DOWNLOAD_STATUS EQUAL 0)
      turbo_message(FATAL_ERROR "Failed to download slang: ${DOWNLOAD_STATUS}")
   endif ()
endif()

if (NOT EXISTS ${SLANG_ROOT}/lib/libslang-compiler.so.0.${SLANG_VERSION})
   if (EXISTS ${SLANG_ROOT}/${SLANG_ARCHIVE})
      turbo_message(STATUS "Extracting ${SLANG_ARCHIVE}...")
      file(
         ARCHIVE_EXTRACT
         INPUT ${SLANG_ROOT}/${SLANG_ARCHIVE}
         DESTINATION ${SLANG_ROOT}
      )
   endif()
endif()

add_library(slang SHARED IMPORTED GLOBAL)
target_include_directories(slang INTERFACE ${SLANG_ROOT}/include)

if (WIN32)
   set_target_properties(slang PROPERTIES
      IMPORTED_IMPLIB "${SLANG_ROOT}/lib/slang-compiler.lib"
		IMPORTED_LOCATION "${SLANG_ROOT}/bin/slang-compiler.dll"
   )
   set_target_properties(slang PROPERTIES
		IMPORTED_LOCATION "${SLANG_ROOT}/bin/slang-glslang.dll"
   )
else()
   set_target_properties(slang PROPERTIES
      IMPORTED_LOCATION ${SLANG_ROOT}/lib/libslang-compiler.so.0.${SLANG_VERSION}
      IMPORTED_NO_SONAME TRUE
   )
endif()
