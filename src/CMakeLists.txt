# ---- library definition ----

if(tinydb_MODULE)
  target_sources(tinydb_lib_module
    PUBLIC
    FILE_SET CXX_MODULES FILES
    lib.cpp
  )
  target_link_libraries(tinydb_lib_module
    PRIVATE
    tinydb_compile_options
  )
else()
  target_sources(tinydb_lib
    PUBLIC
    FILE_SET HEADERS FILES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/lib.hpp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tinydb/lib.hpp>
    PRIVATE
    lib.cpp
  )
  target_link_libraries(tinydb_lib
    PRIVATE
    tinydb_compile_options
  )
endif()

# ---- testing ----

if(BUILD_TESTING)
  include(GoogleTest)
  find_package(GTest REQUIRED)
  add_executable(tinydb_lib_test)
  target_sources(tinydb_lib_test
    PRIVATE
    lib.test.cpp
  )
  if(tinydb_MODULE)
    target_link_libraries(tinydb_lib_test
      PRIVATE
      tinydb_lib_module
      tinydb_compile_options
      Catch2::Catch2WithMain
    )
  else()
    target_link_libraries(tinydb_lib_test
      PRIVATE
      tinydb_lib
      tinydb_compile_options
      GTest::gtest_main
    )
  endif()
  target_precompile_headers(tinydb_lib_test
    PRIVATE
    <gtest/gtest.h>
  )
  gtest_discover_tests(tinydb_lib_test)
endif()
