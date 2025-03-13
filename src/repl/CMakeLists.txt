add_library(tinydb_stmt)
target_module_if_enabled(tinydb_stmt
  MODULES
  stmt.cpp
  SOURCES
  stmt.cpp
)
target_link_libraries(tinydb_stmt
  PRIVATE
  tinydb_compile_options
  fmt::fmt
)

add_executable(tinydb_repl)
target_sources(tinydb_repl
  PRIVATE
  repl.cpp
)
find_package(fmt REQUIRED)
target_link_libraries(tinydb_repl
  PRIVATE
  tinydb_compile_options
  tinydb
  tinydb_stmt
  fmt::fmt
)
if(tinydb_COV)
  add_dependencies(tinydb_repl rm_cov)
endif()
