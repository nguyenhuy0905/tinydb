include(${PROJECT_SOURCE_DIR}/cmake/ModuleIfEnabled.cmake)

add_library(tinydb_sql)
target_sources(tinydb_sql
    PRIVATE
    interpreter.cxx
    tokenizer.cxx
    PUBLIC FILE_SET HEADERS FILES
    tokenizer.hxx
    interpreter.hxx
)

target_link_libraries(tinydb_sql PRIVATE tinydb_compile_opts)
target_include_directories(tinydb_sql PUBLIC ${CMAKE_CURRENT_LIST_DIR})
