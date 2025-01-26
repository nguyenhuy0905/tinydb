add_library(tinydb_dbfile_internal)
target_sources(tinydb_dbfile_internal PUBLIC FILE_SET
    CXX_MODULES FILES
    page.cxx
    page_base.cxx
    page_meta.cxx
    page_serialize.cxx
    btree.cxx
    freelist.cxx
    tbl.cxx
    heap.cxx
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
