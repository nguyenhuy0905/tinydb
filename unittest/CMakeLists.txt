# CMake's default GTest finding
enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(tinydb_unittest
    tbl-test.cxx
    init-test.cxx
    dbfile-test.cxx
)

target_link_libraries(tinydb_unittest
    PRIVATE
    # tinydb_lib
    GTest::gtest_main
    tinydb_tbl
    tinydb_freelist
    tinydb_general
    tinydb_dbfile
)
gtest_discover_tests(tinydb_unittest)
message(STATUS "After building, run ${CMAKE_CURRENT_BINARY_DIR}/tinydb_unittest")
