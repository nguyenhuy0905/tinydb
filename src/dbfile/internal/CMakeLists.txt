add_library(tinydb_dbfile_internal)
target_sources(tinydb_dbfile_internal PUBLIC FILE_SET
    CXX_MODULES FILES
    page.cxxm
    page_base.cxxm
    page_meta.cxxm
    page_serialize.cxxm
    # btree.cxx
    freelist.cxxm
    tbl.cxxm
    heap.cxxm
)
target_link_libraries(tinydb_dbfile_internal
    PUBLIC
    tinydb_general
    tinydb_coltype
    PRIVATE
    tinydb_compile_opts
)
if(tinydb_ENABLE_UNIT_TEST)
    add_subdirectory(test)
endif()
