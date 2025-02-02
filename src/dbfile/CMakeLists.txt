add_library(tinydb_coltype OBJECT)
target_module_if_enabled(tinydb_coltype
  HEADERS
  coltype.hxx
  SOURCES
  coltype.cxx
  MODULES
  coltype.cxx
)
target_link_libraries(tinydb_coltype PRIVATE tinydb_compile_opts)

add_subdirectory(internal)
add_library(tinydb_dbfile OBJECT)
# target_sources(tinydb_dbfile PUBLIC FILE_SET
#     CXX_MODULES FILES dbfile.cxx)
target_module_if_enabled(tinydb_dbfile
  SOURCES
  dbfile.cxx
  MODULES
  dbfile.cxx
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
