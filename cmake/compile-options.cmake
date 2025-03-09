include(CMakeDependentOption)
# compilation options and features

add_library(tinydb_compile_options INTERFACE)
# ---- feature ----
target_compile_features(tinydb_compile_options
  INTERFACE
  cxx_std_${CMAKE_CXX_STANDARD}
)

# ---- warnings ----
cmake_dependent_option(tinydb_WARNINGS "Whether to use compiler warnings" ON
  "PROJECT_IS_TOP_LEVEL;tinydb_DEV" OFF)
if(tinydb_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tinydb_compile_options
      INTERFACE
      -Wall -Wpedantic -Werror -Wextra -Wconversion -Wshadow -Wunused
      -Wsign-conversion -Wcast-qual -Wformat=2 -Wundef -Wnull-dereference
      -Wimplicit-fallthrough -Wnon-virtual-dtor -Wold-style-cast)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(tinydb_compile_options
      INTERFACE
    /W4 /permissive- /volatile:iso /Zc:inline /Zc:preprocessor /Zc:enumTypes
    /Zc:lambda /Zc:__cplusplus /Zc:externConstexpr /Zc:throwingNew /EHsc)
  endif()
endif()

# ---- sanitizers ----
cmake_dependent_option(tinydb_ASAN "Whether to link with AddressSanitizer" ON
  "PROJECT_IS_TOP_LEVEL;
  NOT tinydb_MSAN;
  NOT tinydb_TSAN;
  NOT CMAKE_CXX_COMPILER_ID STREQUAL MSVC;
  tinydb_DEV" OFF)
set(sanitizer_list "")
if(tinydb_ASAN)
  list(APPEND sanitizer_list "address")
endif()
# MSan, TSan and ASan are mutually exclusive
# MSan, TSan and UBSan are clang-and-gnu specific
cmake_dependent_option(tinydb_MSAN "Whether to link with MemorySanitizer" OFF
  "PROJECT_IS_TOP_LEVEL;
  NOT tinydb_ASAN;
  NOT tinydb_TSAN;
  tinydb_DEV;
  NOT CMAKE_CXX_COMPILER_ID STREQUAL MSVC" OFF)
cmake_dependent_option(tinydb_TSAN "Whether to link with ThreadSanitizer" OFF
  "PROJECT_IS_TOP_LEVEL;
  NOT tinydb_ASAN;
  NOT tinydb_MSAN;
  tinydb_DEV;
  NOT CMAKE_CXX_COMPILER_ID STREQUAL MSVC" OFF)
cmake_dependent_option(tinydb_UBSAN "Whether to link with"
  "UndefinedBehaviorSanitizer" ON "PROJECT_IS_TOP_LEVEL;
  tinydb_DEV;
  NOT CMAKE_CXX_COMPILER_ID STREQUAL MSVC" OFF)
if(tinydb_MSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND sanitizer_list "memory")
  endif()
endif()
if(tinydb_TSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND sanitizer_list "thread")
  endif()
endif()
if(tinydb_UBSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND sanitizer_list "undefined")
  endif()
endif()
if(tinydb_ASAN OR tinydb_MSAN OR tinydb_TSAN OR tinydb_UBSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(JOIN sanitizer_list "," sanitizer_opts)
    target_compile_options(tinydb_compile_options
      INTERFACE
      "-fsanitize=${sanitizer_opts}"
    )
    target_link_options(tinydb_compile_options
        INTERFACE "-fsanitize=${sanitizer_opts}")
  endif()
endif()

# ---- PCH ----
option(tinydb_PCH "Whether to build with PCH" OFF)
if(tinydb_PCH)
  target_precompile_headers(tinydb_compile_options
    INTERFACE
    # add some more here, if you need!
    <iostream> <memory> <print>
  )
endif()

# ---- modules ----

cmake_dependent_option(tinydb_MODULE "Whether to build using modules" OFF
  "CMAKE_VERSION VERSION_GREATER_EQUAL 3.28;
  CMAKE_CXX_STANDARD GREATER_EQUAL 20;
  CMAKE_GENERATOR STREQUAL Ninja OR
  CMAKE_GENERATOR MATCHES \"Visual Studio\"" OFF
)
cmake_dependent_option(tinydb_IMPORT_STD "Whether to use import std" OFF
  "CMAKE_CXX_STANDARD GREATER_EQUAL 23;tinydb_MODULE" OFF
)
if(tinydb_MODULE)
  include(GenerateExportHeader)
  target_compile_definitions(tinydb_compile_options INTERFACE TINYDB_MODULE)
  add_library(tinydb_lib_module)
  generate_export_header(tinydb_lib_module
    BASE_NAME tinydb
    EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/tinydb_export.h
  )
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # otherwise, nasty GCC module bug.
    # to be honest, even doing the other way doesn't fix it.
  endif()
  if(tinydb_IMPORT_STD)
    target_compile_definitions(tinydb_compile_options INTERFACE TINYDB_IMPORT_STD)
  endif()
endif()

# ---- coverage ----
cmake_dependent_option(tinydb_COV "Whether to link with coverage" ON
  "PROJECT_IS_TOP_LEVEL;tinydb_DEV" OFF)
if(tinydb_COV)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
      OR
      CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tinydb_compile_options INTERFACE --coverage)
    target_link_options(tinydb_compile_options INTERFACE --coverage)
    include(cmake/coverage.cmake)
  endif()
endif()
