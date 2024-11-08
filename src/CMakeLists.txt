include(../cmake/ModuleIfEnabled.cmake)

add_executable(tinydb)
target_sources(tinydb
    PRIVATE
    main.cxx)
target_link_libraries(tinydb PRIVATE tinydb_compile_opts)
