add_executable(tinydbcli)
target_sources(tinydbcli
    PRIVATE
    main.cxx
    cli.cxx
    PUBLIC FILE_SET HEADERS FILES
    cli.hxx)
target_link_libraries(tinydbcli PRIVATE tinydb_sql tinydb_compile_opts)
