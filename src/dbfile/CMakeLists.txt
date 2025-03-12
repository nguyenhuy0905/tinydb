add_library(tinydb_coltype)
target_module_if_enabled(tinydb_coltype
  HEADERS
  coltype.hxx
  SOURCES
  coltype.cxx
  MODULES
  coltype.cxx
)
target_link_libraries(tinydb_coltype
  PUBLIC
  tinydb_dbfile_internal_base
  PRIVATE
  tinydb_compile_opts
)

add_subdirectory(internal)
add_library(tinydb_dbfile)
# target_sources(tinydb_dbfile PUBLIC FILE_SET
#     CXX_MODULES FILES dbfile.cxx)
target_module_if_enabled(tinydb_dbfile
  HEADERS
  dbfile.hxx
  tbl_builder.hxx
  SOURCES
  dbfile.cxx
  tbl_builder.cxx
  MODULES
  dbfile.cxx
  tbl_builder.cxx
)
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
