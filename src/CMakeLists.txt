include(GenerateExportHeader)
add_library(tinydb)
target_sources(tinydb
    PUBLIC FILE_SET CXX_MODULES FILES
    tinydb.cxx
)
generate_export_header(tinydb)
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
    include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)
    find_package(GTest)
    if(NOT GTest_FOUND)
        CPMAddPackage(
            GTest
            URL https://github.com/google/googletest/releases/download/v1.15.0/googletest-1.15.0.tar.gz
        )
    endif()
    # target_sources this one.
    add_executable(tinydb_test)
    target_link_libraries(tinydb_test
        PRIVATE
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(tinydb_test)
endif()

add_subdirectory(test)
add_subdirectory(general)
add_subdirectory(sql)
add_subdirectory(dbfile)
add_subdirectory(cli)
