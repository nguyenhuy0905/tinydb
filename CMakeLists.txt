# ##############################################################################
# tinydb top level CMakeLists
# ##############################################################################

# module support starts at 3.28
cmake_minimum_required(VERSION 3.28)
project(
    tinydb
    VERSION 0.1.0
    LANGUAGES C CXX
    HOMEPAGE_URL
        "https://github.com/huynguyen-and-friend-projects/cmake-template"
        DESCRIPTION "Somewhat portable CMake project template")

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

# turn off compiler extensions
message(STATUS "Turn off ${CMAKE_CXX_COMPILER_ID} extensions")
message(STATUS "Turn off ${CMAKE_C_COMPILER_ID} extensions")
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
add_subdirectory(submodule)

if(tinydb_ENABLE_UNIT_TEST)
    message(STATUS "Unit test enabled")
    add_subdirectory(unittest)
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
    message(STATUS "Doxygen enabled. Generating docs...")
    include(cmake/Doxygen.cmake)
endif()
