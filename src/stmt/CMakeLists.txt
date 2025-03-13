add_library(tinydb_stmt)
target_module_if_enabled(tinydb_stmt
  MODULES
  stmt.cpp
  SOURCES
  stmt.cpp
)
find_package(fmt REQUIRED)
target_link_libraries(tinydb_stmt
  PRIVATE
  tinydb_compile_options
  fmt::fmt
)
