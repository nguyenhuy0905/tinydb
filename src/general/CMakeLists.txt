add_library(tinydb_general INTERFACE)
configure_file(version.hxx.in version.hxx)
target_include_directories(tinydb_general
    INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})
