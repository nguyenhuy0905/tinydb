include(../cmake/ModuleIfEnabled.cmake)

add_executable(tinydb)
target_sources(tinydb
    PRIVATE
    main.cxx)

add_library(tinydb_tokenizer)
target_sources(tinydb_tokenizer
    PRIVATE
    tokenizer.cxx
    PRIVATE FILE_SET HEADERS FILES tokenizer.hxx)
target_link_libraries(tinydb_tokenizer PRIVATE tinydb_compile_opts)

add_library(tinydb_cli)
target_sources(tinydb_cli
    PRIVATE
    cli.cxx
    PRIVATE FILE_SET HEADERS FILES cli.hxx)
target_link_libraries(tinydb_cli PRIVATE tinydb_compile_opts PUBLIC tinydb_tokenizer)

target_link_libraries(tinydb PRIVATE tinydb_compile_opts tinydb_cli)
