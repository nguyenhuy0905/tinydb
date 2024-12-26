# will stay as one-library-one-file until the code is stabilized.

add_library(tinydb_page)
target_sources(tinydb_page
    PRIVATE
    page.cxx
    PUBLIC FILE_SET HEADERS FILES
    page.hxx
)
target_link_libraries(tinydb_page PRIVATE tinydb_compile_opts)

add_library(tinydb_freelist)
target_sources(tinydb_freelist
    PRIVATE
    freelist.cxx
    PUBLIC FILE_SET HEADERS FILES
    freelist.hxx
)
target_link_libraries(tinydb_freelist PRIVATE tinydb_compile_opts PUBLIC tinydb_page)

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
