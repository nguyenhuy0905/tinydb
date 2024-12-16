include(${PROJECT_SOURCE_DIR}/cmake/ModuleIfEnabled.cmake)

add_executable(tinydb)
target_sources(tinydb
    PRIVATE
    main.cxx
    cli.cxx
    interpreter.cxx
    tokenizer.cxx
    PUBLIC FILE_SET HEADERS FILES
    cli.hxx
    tokenizer.hxx
    interpreter.hxx
)

target_link_libraries(tinydb PRIVATE tinydb_compile_opts)
