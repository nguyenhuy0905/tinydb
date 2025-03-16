add_library(tinydb_stmt)
target_module_if_enabled(tinydb_stmt
  HEADERS
  stmt.hpp
  token.hpp
  MODULES
  stmt.cpp
  token.cpp
  SOURCES
  stmt.cpp
  token.cpp
)
find_package(fmt REQUIRED)
target_link_libraries(tinydb_stmt
  PRIVATE
  tinydb_compile_options
  fmt::fmt
)

if(BUILD_TESTING)
  target_sources(tinydb_test
    PRIVATE
    token.test.cpp
  )
endif()
