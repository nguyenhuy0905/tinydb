add_subdirectory(internal)
add_library(tinydb_dbfile)
target_sources(tinydb_dbfile PUBLIC FILE_SET
    CXX_MODULES FILES dbfile.cxx)
target_link_libraries(tinydb_dbfile
    PRIVATE
    tinydb_compile_opts
    tinydb_dbfile_internal
    PUBLIC
    tinydb_general
)

if(tinydb_ENABLE_UNIT_TEST)
    add_subdirectory(test)
endif()
