# will stay as one-library-one-file until the code is stabilized.

add_library(tinydb_page)
# target_sources(tinydb_page
#     PRIVATE
#     page.cxx
#     PUBLIC FILE_SET HEADERS FILES
#     page.hxx
# )
tinydb_target_module_if_enabled(tinydb_page
    HEADER_FILES
    page.hxx
    MODULE_FILES
    page.cxx
    SOURCE_FILES
    page.cxx
)
target_link_libraries(tinydb_page PRIVATE tinydb_compile_opts PUBLIC tinydb_general)

add_library(tinydb_freelist)
# target_sources(tinydb_freelist
#     PRIVATE
#     freelist.cxx
#     PUBLIC FILE_SET HEADERS FILES
#     freelist.hxx
# )
tinydb_target_module_if_enabled(tinydb_freelist
    HEADER_FILES
    freelist.hxx
    MODULE_FILES
    freelist.cxx
    SOURCE_FILES
    freelist.cxx
)
target_link_libraries(tinydb_freelist PRIVATE tinydb_compile_opts PUBLIC tinydb_page)

add_library(tinydb_tbl)
# target_sources(tinydb_tbl
#     PRIVATE
#     tbl.cxx
#     PUBLIC FILE_SET HEADERS FILES
#     tbl.hxx
# )
tinydb_target_module_if_enabled(tinydb_tbl
    HEADER_FILES
    tbl.hxx
    MODULE_FILES
    tbl.cxx
    SOURCE_FILES
    tbl.cxx
)
target_link_libraries(tinydb_tbl PUBLIC tinydb_page PRIVATE tinydb_compile_opts)

add_library(tinydb_tree)
target_sources(tinydb_tree
    PRIVATE
    btree.cxx
    PUBLIC FILE_SET HEADERS FILES
    btree.hxx
)
target_link_libraries(tinydb_tree PRIVATE tinydb_compile_opts PUBLIC tinydb_tbl)

add_library(tinydb_dbfile)
# target_sources(tinydb_dbfile
#     PRIVATE
#     dbfile.cxx
#     PUBLIC FILE_SET HEADERS FILES
#     dbfile.hxx)
tinydb_target_module_if_enabled(tinydb_dbfile
    HEADER_FILES
    dbfile.hxx
    MODULE_FILES
    dbfile.cxx
    SOURCE_FILES
    dbfile.cxx
)
target_link_libraries(tinydb_dbfile
    PRIVATE
    tinydb_compile_opts
    PUBLIC
    tinydb_page
    tinydb_freelist
    tinydb_tbl
    tinydb_tree)
