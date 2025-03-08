include(cmake/no-insource.cmake)
cmake_minimum_required(VERSION 3.28.0)
if(CMAKE_VERSION VERSION_GREATER 3.30 AND template_IMPORT_STD)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # This specific value changes as experimental support evolves. See
  # `Help/dev/experimental.rst` in the CMake source corresponding to
  # your CMake build for the exact value to use.
  "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
endif()

project(template
    VERSION 0.1.0
    HOMEPAGE_URL "https://example.com"
    DESCRIPTION "Another CMake template"
    LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

include(cmake/top-level.cmake)
include(cmake/compile-options.cmake)
include(cmake/folders.cmake)

# ---- library ----

# target_sources inside the src/ directory

if(NOT template_MODULE)
  add_library(template_lib)
  target_include_directories(template_lib ${WARNING_GUARD}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
  add_library(template::template ALIAS template_lib)
endif()

include(CMakeDependentOption)
cmake_dependent_option(template_DEV "Enable if you plan to work on this project"
    OFF "PROJECT_IS_TOP_LEVEL" OFF
)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
include(cmake/docs.cmake)
if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install.cmake)
endif()
