include(${PROJECT_SOURCE_DIR}/cmake/ModuleIfEnabled.cmake)
include(GenerateExportHeader)
add_library(tinydb)
tinydb_target_module_if_enabled(tinydb
    MODULE_FILES tinydb.cxx
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

add_subdirectory(general)
add_subdirectory(sql)
add_subdirectory(dbfile)
add_subdirectory(cli)
