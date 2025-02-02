# ##############################################################################
# tinydb top level CMakeLists
# ##############################################################################

# module support starts at 3.28, but we need 3.30 to import std
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
project(
    tinydb
    VERSION 0.0.1
    LANGUAGES C CXX
    HOMEPAGE_URL
        "https://github.com/nguyenhuy0905/tinydb"
        DESCRIPTION "A non-functioning database as of now")

if(NOT CMAKE_CXX_STANDARD)
  message(STATUS "C++ standard not set, default to C++23")
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(NOT CMAKE_C_STANDARD AND NOT MSVC)
  message(STATUS "C standard not set, default to C23")
  set(CMAKE_C_STANDARD 23)
  set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# hide as much as I can
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(tinydb_compile_opts INTERFACE)
target_compile_features(tinydb_compile_opts
                        INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

message(STATUS "Configure project options")
include(CMakeOptions.cmake)

if(NOT MSVC)
  # there's no such thing as a "C standard" in MSVC
  target_compile_features(tinydb_compile_opts
                            INTERFACE c_std_${CMAKE_C_STANDARD})
endif()

if(tinydb_PACK)
  include(CPack)
  include(cmake/PackConfig.cmake)
endif()

add_subdirectory(src)

if(tinydb_ENABLE_UNIT_TEST)
  message(STATUS "Unit test enabled")
endif()

if(tinydb_ENABLE_FUZZ_TEST)
  message(STATUS "Fuzz test enabled")
  message(STATUS "Checking if libFuzzer linkable")
  include(cmake/CheckFuzzSourceCompile.cmake)
  tinydb_check_fuzz_compile(tinydb_FUZZ_COMPILE)
  if(tinydb_FUZZ_COMPILE)
    add_subdirectory(fuzztest)
  endif()
endif()

if(tinydb_ENABLE_BENCHMARK)
  message(STATUS "Benchmark enabled")
  add_subdirectory(benchmark)
endif()

if(tinydb_ENABLE_DOXYGEN)
  message(STATUS "Doxygen enabled")
  include(cmake/Doxygen.cmake)
endif()
