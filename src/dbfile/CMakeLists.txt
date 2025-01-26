add_library(tinydb_coltype)
target_sources(tinydb_coltype PUBLIC FILE_SET CXX_MODULES FILES coltype.cxxm)
target_link_libraries(tinydb_coltype PRIVATE tinydb_compile_opts)

add_subdirectory(internal)
add_library(tinydb_dbfile)
target_sources(tinydb_dbfile PUBLIC FILE_SET
    CXX_MODULES FILES dbfile.cxxm)
target_link_libraries(tinydb_dbfile
    PRIVATE
    tinydb_compile_opts
    tinydb_dbfile_internal
    PUBLIC
    tinydb_general
    tinydb_coltype
)

if(tinydb_ENABLE_UNIT_TEST)
    add_subdirectory(test)
endif()
