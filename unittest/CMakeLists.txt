# CMake's default GTest finding
enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

add_executable(tinydb_unittest
    unit-test.cxx)

target_link_libraries(tinydb_unittest
    PRIVATE
    # tinydb_lib
    GTest::gtest_main
    tinydb_tbl)
gtest_discover_tests(tinydb_unittest)
message(STATUS "After building, run ${CMAKE_CURRENT_BINARY_DIR}/tinydb_unittest")
