# ---- convenient functions ----

# target_sources for an already-added library target.
# if tinydb_MODULE, target modules.
# otherwise, don't.
#
# a module build assumes no header.
# options:
# - HEADER_ONLY: if the library is built not as a module, make it an interface
# library and only target headers. Otherwise, this option is ignored.
# This option also ignores SOURCES.
#   - If you want a header-only library regardless whether module is enabled,
#   just simply add_library and target_sources yourself.
# - HEADERS/MODULES/SOURCES: list of files of each category.
#   - SOURCES should never be empty. If your module and source files are the
#   same (say, you use the macro #TINYDB_MODULE to enable/disable certain
#   sections), just repeat the file name.
#
function(target_module_if_enabled target)
  cmake_parse_arguments(source "HEADER_ONLY" ""
    "HEADERS;MODULES;SOURCES"  ${ARGN})
  if(source_HEADER_ONLY AND NOT source_HEADERS)
    message(FATAL_ERROR "Target ${target} declared header-only,\
      but no header is supplied")
  endif()

  if(source_HEADER_ONLY AND NOT tinydb_MODULE)
    target_sources(${target}
      INTERFACE FILE_SET HEADERS FILES ${source_HEADERS}
    )
    return()
  endif()

  if(tinydb_MODULE)
    target_sources(${target}
      PUBLIC FILE_SET CXX_MODULES FILES ${source_MODULES}
    )
  else()
    if(source_HEADERS)
      target_sources(${target}
        PUBLIC FILE_SET HEADERS FILES ${source_HEADERS}
      )
    endif()
  endif()
  if(NOT source_SOURCES)
    message(FATAL_ERROR "Target ${target} has no source file")
  endif()
  target_sources(${target}
    PRIVATE
    ${source_SOURCES}
  )
endfunction()

# ---- library definition ----

add_subdirectory(stmt)
target_module_if_enabled(tinydb
  HEADERS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tinydb.hpp>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tinydb/tinydb.hpp>
  MODULES
  tinydb.cpp
  SOURCES
  tinydb.cpp
)
target_link_libraries(tinydb
  PRIVATE
  tinydb_compile_options
)

# repl executable
add_subdirectory(repl)

# ---- testing ----

if(BUILD_TESTING)
  include(GoogleTest)
  find_package(GTest REQUIRED)
  add_executable(tinydb_test)
  target_sources(tinydb_test
    PRIVATE
    tinydb.test.cpp
  )
  target_link_libraries(tinydb_test
    PRIVATE
    tinydb
    tinydb_compile_options
    GTest::gtest_main
  )
  gtest_discover_tests(tinydb_test)
endif()
