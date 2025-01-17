# CMake's default GTest finding
enable_testing()
include(GoogleTest)
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)
CPMAddPackage(NAME
    GTest
    URL https://github.com/google/googletest/releases/download/v1.15.0/googletest-1.15.0.tar.gz
)
find_package(GTest REQUIRED)

add_executable(tinydb_unittest
    tbl-test.cxx
    init-test.cxx
    dbfile-test.cxx
)

target_link_libraries(tinydb_unittest
    PRIVATE
    tinydb_compile_opts
    GTest::gtest_main
    tinydb_dbfile
)
gtest_discover_tests(tinydb_unittest)
message(STATUS "After building, run ${CMAKE_CURRENT_BINARY_DIR}/tinydb_unittest")
