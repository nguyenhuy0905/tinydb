if(tinydb_ENABLE_UNIT_TEST)
    # add GTest::gtest_main or anything of GTest::
    # as a link library for test targets.
    include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)
    find_package(GTest)
    if(NOT GTest_FOUND)
        CPMAddPackage(
            GTest
            URL https://github.com/google/googletest/releases/download/v1.15.0/googletest-1.15.0.tar.gz
        )
    endif()
endif()

add_subdirectory(internal)
add_library(tinydb_dbfile)
tinydb_target_module_if_enabled(tinydb_dbfile
    HEADER_FILES
    dbfile.hxx
    coltype.hxx
    MODULE_FILES
    dbfile.cxx
    SOURCE_FILES
    dbfile.cxx
)
target_link_libraries(tinydb_dbfile
    PRIVATE
    tinydb_compile_opts
    tinydb_dbfile_internal
    PUBLIC
    tinydb_general
)
