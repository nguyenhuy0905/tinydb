#
# List of options
#
include(CMakeDependentOption)

# if build type is not set
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("No build type specified! Default to Debug")
    set(CMAKE_BUILD_TYPE
        "Debug"
        CACHE STRING
              "Choose a build type (Debug;Release;MinSizeRel;RelWithDebInfo)")
    set_property(CACHE CMAKE_BUILD_TYPE
                 PROPERTY STRINGS "Debug;Release;MinSizeRel;RelWithDebInfo")
endif()

#
# Global options
#

# If there's another top-level project, high chance that project already sets
# these options.
option(tinydb_ENABLE_CCACHE "Use ccache" OFF)
option(tinydb_ENABLE_LLD "Use lld instead of the compiler-default linker" OFF)
option(tinydb_ENABLE_LTO "Enable link-time optimization" OFF)
option(tinydb_ENABLE_WARNING "Turn on compiler warnings." ON)
cmake_dependent_option(
    tinydb_WARNING_AS_ERR "Turn compiler warnings into errors" OFF
    "tinydb_ENABLE_WARNING" OFF)

#
# Setup for this project only
#

option(tinydb_ENABLE_COVERAGE "Add coverage flags" OFF)
option(tinydb_ENABLE_PCH "Use precompiled headers" OFF)
option(tinydb_ENABLE_DOXYGEN "Use Doxygen to generate documents" OFF)
cmake_dependent_option(tinydb_USE_IMPORT_STD "Whether to use 'import std'" ON
    "CMAKE_CXX_MODULE_STD" OFF)
# option(tinydb_USE_IMPORT_STD "Whether to use 'import std'" OFF)
# in my experience, turning this off gives better debug information
cmake_dependent_option(
    tinydb_ENABLE_DEBUG_OPTIMIZATION
    "Turn this off to disable optimization (if any) in Debug builds" ON
    "\"${CMAKE_BUILD_TYPE}\" STREQUAL \"Debug\"" ON)
option(tinydb_ENABLE_ASAN "Link libASan to executable" OFF)
option(tinydb_ENABLE_UBSAN "Link libUBSan to executable" OFF)
option(tinydb_ENABLE_MSAN "Link libMSan to executable" OFF)
option(tinydb_ENABLE_TSAN "Link libTSan to executable" OFF)

# options to include more subdirs
option(tinydb_ENABLE_UNIT_TEST
       "Build unit test executable. Requires GoogleTest." OFF)
option(tinydb_ENABLE_FUZZ_TEST
       "Build fuzz test executable. Requires libFuzzer." OFF)
option(tinydb_ENABLE_HARDENING "Add hardening flags to the compiler" OFF)
option(
    tinydb_ENABLE_BENCHMARK
    "Build micro-benchmark executable.
    Requires GoogleTest to be installed alongside Google Benchmark."
    OFF)

# installing and packing
cmake_dependent_option(tinydb_INSTALL "Configure installation for this project"
                       OFF "PROJECT_IS_TOP_LEVEL" OFF)
cmake_dependent_option(tinydb_PACK "Configure packing for this project" OFF
                       "PROJECT_IS_TOP_LEVEL" OFF)

if(NOT PROJECT_IS_TOP_LEVEL)
    message(STATUS "Project is not top-level.")
    mark_as_advanced(
        tinydb_ENABLE_CCACHE
        tinydb_ENABLE_LLD
        tinydb_ENABLE_LTO
        tinydb_ENABLE_WARNING
        tinydb_WARNING_AS_ERR
        tinydb_ENABLE_UNIT_TEST
        tinydb_ENABLE_FUZZ_TEST
        tinydb_ENABLE_BENCHMARK
        tinydb_ENABLE_OPTIMIZATION
        tinydb_ENABLE_COVERAGE
        tinydb_ENABLE_PCH
        tinydb_ENABLE_DOXYGEN
        tinydb_ENABLE_ASAN
        tinydb_ENABLE_UBSAN
        tinydb_ENABLE_MSAN
        tinydb_ENABLE_TSAN
        tinydb_INSTALL
        tinydb_PACK)
else()
    message(STATUS "Project is top-level. Configuring global options.")
    include(cmake/GlobalConfig.cmake)
    tinydb_global_config()
endif()

include(cmake/ProjectConfig.cmake)
message(STATUS "Configuring project-specific options.")
if(tinydb_ENABLE_PCH)
    set_pch_options(PCH <memory> <string> <print> <expected> <variant> <vector>
        <ranges> <iostream> <fstream>)
endif()
tinydb_local_config()

# this has to stay here for ctest to more conveniently work
if(tinydb_ENABLE_UNIT_TEST)
    include(CTest)
    # Probably gonna have to break this down once the codebase grows.
    # For now I only need to test dbfile.
    add_test(NAME tinydb_test COMMAND ${CMAKE_BINARY_DIR}/src/tinydb_test)
endif()
