include(GenerateExportHeader)
include(${PROJECT_SOURCE_DIR}/cmake/ModuleIfEnabled.cmake)
target_module_if_enabled(tinydb
  MODULES
  tinydb.cxx
  SOURCES
  tinydb.cxx
)
target_include_directories(tinydb_compile_opts
    INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(tinydb
    PRIVATE
    tinydb_compile_opts
    PUBLIC
    tinydb_dbfile)

# unit test
if(tinydb_ENABLE_UNIT_TEST)
  find_package(GTest REQUIRED)
  # target_sources this one.
  add_executable(tinydb_test)
  target_link_libraries(tinydb_test
        PRIVATE
        GTest::gtest_main
  )
  if(tinydb_ENABLE_PCH)
    target_precompile_headers(tinydb_test PRIVATE <gtest/gtest.h>)
  endif()
  include(GoogleTest)
  gtest_discover_tests(tinydb_test)
endif()

add_subdirectory(general)
add_subdirectory(sql)
add_subdirectory(dbfile)
add_subdirectory(cli)
