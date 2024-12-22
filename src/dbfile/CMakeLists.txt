add_library(tinydb_tbl)
target_sources(tinydb_tbl
    PRIVATE
    tbl.cxx
    PUBLIC FILE_SET HEADERS FILES
    tbl.hxx
)
target_link_libraries(tinydb_tbl PRIVATE tinydb_compile_opts)

add_library(tinydb_tree)
target_sources(tinydb_tree
    PRIVATE
    btree.cxx
    PUBLIC FILE_SET HEADERS FILES
    btree.hxx
)
target_link_libraries(tinydb_tree PRIVATE tinydb_compile_opts PUBLIC tinydb_tbl)
