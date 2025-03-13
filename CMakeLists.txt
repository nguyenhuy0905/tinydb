include(cmake/no-insource.cmake)
cmake_minimum_required(VERSION 3.28.0)
if(CMAKE_VERSION VERSION_GREATER 3.30 AND tinydb_IMPORT_STD)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
endif()

project(tinydb
    VERSION 0.1.0
    HOMEPAGE_URL "https://example.com"
    DESCRIPTION "Supposedly simple database"
    LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_MODULE_STD 1)

include(cmake/top-level.cmake)
include(cmake/compile-options.cmake)
include(cmake/folders.cmake)

# ---- library ----

# target_sources inside the src/ directory

include(GenerateExportHeader)
add_library(tinydb)
generate_export_header(tinydb
  BASE_NAME TINYDB
)
target_include_directories(tinydb_compile_options
  INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
  "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
add_library(tinydb::tinydb ALIAS tinydb)

include(CMakeDependentOption)
cmake_dependent_option(tinydb_DEV "Enable if you plan to work on this project"
    OFF "PROJECT_IS_TOP_LEVEL" OFF
)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
include(cmake/docs.cmake)
if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install.cmake)
endif()
